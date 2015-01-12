/*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements. See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership. The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License. You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied. See the License for the
* specific language governing permissions and limitations
* under the License.
*
* Contains some contributions under the Thrift Software License.
* Please see doc/old-thrift-license.txt in the Thrift distribution for
* details.
*/

#include <cassert>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <sys/stat.h>

#include "platform.h"
#include "t_oop_generator.h"

using std::map;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

static const string endl = "\n";  // avoid ostream << std::endl flushes

/**
* C++ code generator. This is legitimacy incarnate.
*
*/
class t_cppemb_generator : public t_oop_generator {
public:
    t_cppemb_generator(
        t_program* program,
        const std::map<std::string, std::string>& parsed_options,
        const std::string& option_string)
    : t_oop_generator(program)
    {
        (void) option_string;
        std::map<std::string, std::string>::const_iterator iter;
        
        iter = parsed_options.find("include_prefix");
        use_include_prefix_ = (iter != parsed_options.end());
        
        iter = parsed_options.find("embedded_namespace");
        use_embedded_namespace_ = (iter != parsed_options.end());
        
        out_dir_base_ = "gen-cppemb";
    }
    
    /**
    * Init and close methods
    */
    
    void init_generator();
    void close_generator();
    
    void generate_consts(std::vector<t_const*> consts);
    
    /**
    * Program-level generation functions
    */
    
    void generate_typedef(t_typedef* ttypedef);
    void generate_enum(t_enum* tenum);
    void generate_forward_declaration(t_struct* tstruct);
    void generate_struct(t_struct* tstruct) {
        generate_cpp_struct(tstruct, false);
    }
    void generate_xception(t_struct* txception) {
        generate_cpp_struct(txception, true);
    }
    void generate_cpp_struct(t_struct* tstruct, bool is_exception);
    
    void generate_service(t_service* tservice);
    
    void print_const_value(std::ofstream& out, std::string name, t_type* type, t_const_value* value);
    std::string render_const_value(std::ofstream& out, std::string name, t_type* type, t_const_value* value);
    
    void generate_struct_declaration    (std::ofstream& out, t_struct* tstruct,
        bool is_exception=false,
        bool read=true,
        bool write=true);
    void generate_struct_reader        (std::ofstream& out, t_struct* tstruct);
    void generate_struct_reader_fragment(std::ofstream& out, t_struct* tstruct);
    void generate_struct_writer        (std::ofstream& out, t_struct* tstruct);
    void generate_struct_result_writer (std::ofstream& out, t_struct* tstruct);
    
    /**
    * Service-level generation functions
    */
    
    void generate_service_interface (t_service* tservice);
    void generate_service_implementation (t_service* tservice);
    void generate_service_interface_factory (t_service* tservice);
    void generate_service_null      (t_service* tservice);
    void generate_service_client    (t_service* tservice);
    void generate_service_client_implementation    (t_service* tservice);
    void generate_service_skeleton  (t_service* tservice);
    void generate_function_helpers  (t_service* tservice, t_function* tfunction);
    
    /**
    * Serialization constructs
    */
    
    void generate_deserialize_type        (std::ofstream& out,
        t_type*    tfield,
        std::string name);

    void generate_deserialize_field        (std::ofstream& out,
        t_field*    tfield,
        std::string prefix="",
        std::string suffix="");
    
    void generate_deserialize_struct       (std::ofstream& out,
        t_struct*   tstruct,
        std::string prefix="");
    
    void generate_deserialize_container    (std::ofstream& out,
        t_type*     ttype,
        std::string prefix="");
    
    void generate_deserialize_set_element  (std::ofstream& out,
        t_set*      tset,
        std::string prefix="");
    
    void generate_deserialize_map_element  (std::ofstream& out,
        t_map*      tmap,
        std::string prefix="");
    
    void generate_deserialize_list_element (std::ofstream& out,
        t_list*     tlist,
        std::string prefix,
        bool push_back,
        std::string index);
    
    void generate_serialize_type        (std::ofstream& out,
        t_type*    tfield,
        std::string name,
        int32_t field_id
        );

    void generate_serialize_field          (std::ofstream& out,
        t_field*    tfield,
        std::string prefix="",
        std::string suffix="");
    
    void generate_serialize_struct         (std::ofstream& out,
        t_struct*   tstruct,
        std::string prefix="");
    
    void generate_serialize_container      (std::ofstream& out,
        t_type*     ttype,
        std::string prefix="");
    
    void generate_serialize_map_element    (std::ofstream& out,
        t_map*      tmap,
        std::string iter);
    
    void generate_serialize_set_element    (std::ofstream& out,
        t_set*      tmap,
        std::string iter);
    
    void generate_serialize_list_element   (std::ofstream& out,
        t_list*     tlist,
        std::string iter);
    
    void generate_function_call            (ostream& out,
        t_function* tfunction,
        string target,
        string iface,
        string arg_prefix);
    /*
    * Helper rendering functions
    */
    
    std::string namespace_prefix(std::string ns);
    std::string namespace_open(std::string ns);
    std::string namespace_close(std::string ns);
    std::string type_name(t_type* ttype, bool in_typedef=false, bool arg=false);
    std::string base_type_name(t_base_type::t_base tbase);
    std::string declare_field(t_field* tfield, bool init=false, bool constant=false);
    std::string function_signature(t_function* tfunction, std::string prefix="", bool name_params=true);
    std::string argument_list(t_struct* tstruct, bool name_params=true, bool start_comma=false, bool insert_isset = false, bool type_params = true);
    std::string type_to_enum(t_type* ttype);
    
    void generate_enum_constant_list(std::ofstream& f,
        const vector<t_enum_value*>& constants,
        const char* prefix,
        const char* suffix,
        bool include_values);
    
    
    bool is_complex_type(t_type* ttype) {
        ttype = get_true_type(ttype);
        
        return
        ttype->is_container() ||
        ttype->is_struct() ||
        ttype->is_xception() ||
        (ttype->is_base_type() && (((t_base_type*)ttype)->get_base() == t_base_type::TYPE_STRING));
    }
    
private:
/**
* Returns the include prefix to use for a file generated by program, or the
* empty string if no include prefix should be used.
*/
std::string get_include_prefix(const t_program& program) const;

/**
* True iff we should use a path prefix in our #include statements for other
* thrift-generated header files.
*/
bool use_include_prefix_;

/**
* True iff we should use an additional namespace embedded to make using cpp egenerated classes
* and embededd generated class in the same process possible
*/
bool use_embedded_namespace_;

/**
* Strings for namespace, computed once up front then used directly
*/

std::string ns_open_;
std::string ns_close_;

/**
* File streams, stored here to avoid passing them as parameters to every
* function.
*/

std::ofstream f_types_;
std::ofstream f_types_impl_;
std::ofstream f_header_;
std::ofstream f_service_;

};

/**
* Prepares for file generation by opening up the necessary file output
* streams.
*/
void t_cppemb_generator::init_generator() {
    // Make output directory
    MKDIR(get_out_dir().c_str());
    
    // Make output file
    string f_types_name = get_out_dir()+program_name_+"_embedded_types.h";
    f_types_.open(f_types_name.c_str());
    
    string f_types_impl_name = get_out_dir()+program_name_+"_embedded_types.cpp";
    f_types_impl_.open(f_types_impl_name.c_str());
    
    // Print header
    f_types_ <<
    autogen_comment();
    f_types_impl_ <<
    autogen_comment();
    
    // Start ifndef
    f_types_ <<
    "#ifndef " << program_name_ << "_EMBEDDED_TYPES_H" << endl <<
    "#define " << program_name_ << "_EMBEDDED_TYPES_H" << endl <<
    endl;
    
    // Include base types
    f_types_ <<
    "#include <cstdint>" << endl <<
    "#include \"General.h\"" << endl <<
    "#include \"ThriftBase.hxx\"" << endl <<
    endl;
    
    // Include other Thrift includes
    const vector<t_program*>& includes = program_->get_includes();
    for (size_t i = 0; i < includes.size(); ++i) {
        f_types_ <<
        "#include \"" << get_include_prefix(*(includes[i])) <<
        includes[i]->get_name() << "_embedded_types.h\"" << endl;
        
    }
    f_types_ << endl;
    
    // Include custom headers
    const vector<string>& cpp_includes = program_->get_cpp_includes();
    for (size_t i = 0; i < cpp_includes.size(); ++i) {
        if (cpp_includes[i][0] == '<') {
            f_types_ <<
            "#include " << cpp_includes[i] << endl;
        } else {
            f_types_ <<
            "#include \"" << cpp_includes[i] << "\"" << endl;
        }
    }
    f_types_ << endl;
    f_types_ << "namespace Thrift { class Reader; class Writer; }" << endl;
    
    // Include the types file
    f_types_impl_ <<
    "#include \"" << get_include_prefix(*get_program()) << program_name_ <<
    "_embedded_types.h\"" << endl <<
    "#include \"ThriftReader.hxx\"" << endl << 
    "#include \"ThriftWriter.hxx\"" << endl << 
    endl;
    
    // Open namespace
    ns_open_ = namespace_open(program_->get_namespace("cpp"));
    ns_close_ = namespace_close(program_->get_namespace("cpp"));
    
    f_types_ <<
    ns_open_ << endl <<
    endl;
    
    f_types_impl_ <<
    ns_open_ << endl <<
    endl;
}

/**
* Closes the output files.
*/
void t_cppemb_generator::close_generator() {
    // Close namespace
    f_types_ <<
    ns_close_ << endl <<
    endl;
    f_types_impl_ <<
    ns_close_ << endl;
    
    // Close ifndef
    f_types_ <<
    "#endif" << endl;
    
    // Close output file
    f_types_.close();
    f_types_impl_.close();
}

/**
* Generates a typedef. This is just a simple 1-liner in C++
*
* @param ttypedef The type definition
*/
void t_cppemb_generator::generate_typedef(t_typedef* ttypedef) {
    f_types_ <<
    indent() << "typedef " << type_name(ttypedef->get_type(), true) << " " << ttypedef->get_symbolic() << ";" << endl <<
    endl;
}


void t_cppemb_generator::generate_enum_constant_list(std::ofstream& f,
    const vector<t_enum_value*>& constants,
    const char* prefix,
    const char* suffix,
    bool include_values)
{
    f << " {" << endl;
    indent_up();
    
    vector<t_enum_value*>::const_iterator c_iter;
    bool first = true;
    for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
        if (first) {
            first = false;
        } else {
            f << "," << endl;
        }
        indent(f)
        << prefix << (*c_iter)->get_name() << suffix;
        if (include_values) {
            f << " = " << (*c_iter)->get_value();
        }
    }
    
    f << endl;
    indent_down();
    indent(f) << "};" << endl;
}

/**
* Generates code for an enumerated type. In C++, this is essentially the same
* as the thrift definition itself, using the enum keyword in C++.
*
* @param tenum The enumeration
*/
void t_cppemb_generator::generate_enum(t_enum* tenum) {
    vector<t_enum_value*> constants = tenum->get_constants();
    
    std::string enum_name = tenum->get_name();
    f_types_ <<
    indent() << "enum class " << enum_name << " : int32_t";
    
    generate_enum_constant_list(f_types_, constants, "", "", true);
    
    indent_down();
    f_types_ << "};" << endl;
    
    f_types_ << endl;
}

/**
* Generates a class that holds all the constants.
*/
void t_cppemb_generator::generate_consts(std::vector<t_const*> consts) {
    string f_consts_name = get_out_dir()+program_name_+"_embedded_constants.h";
    ofstream f_consts;
    f_consts.open(f_consts_name.c_str());
    
    string f_consts_impl_name = get_out_dir()+program_name_+"_embedded_constants.cpp";
    ofstream f_consts_impl;
    f_consts_impl.open(f_consts_impl_name.c_str());
    
    // Print header
    f_consts <<
    autogen_comment();
    f_consts_impl <<
    autogen_comment();
    
    // Start ifndef
    f_consts <<
    "#ifndef " << program_name_ << "_EMBEDDED_CONSTANTS_H" << endl <<
    "#define " << program_name_ << "_EMBEDDED_CONSTANTS_H" << endl <<
    endl <<
    "#include \"" << get_include_prefix(*get_program()) << program_name_ <<
    "_embedded_types.h\"" << endl <<
    endl <<
    ns_open_ << endl <<
    endl;
    
    f_consts_impl <<
    "#include \"" << get_include_prefix(*get_program()) << program_name_ <<
    "_embedded_constants.h\"" << endl <<
    endl <<
    ns_open_ << endl <<
    endl;
    
    f_consts <<
    "class " << program_name_ << "Constants {" << endl <<
    " public:" << endl <<
    "  " << program_name_ << "Constants();" << endl <<
    endl;
    indent_up();
    vector<t_const*>::iterator c_iter;
    for (c_iter = consts.begin(); c_iter != consts.end(); ++c_iter) {
        string name = (*c_iter)->get_name();
        t_type* type = (*c_iter)->get_type();
        f_consts <<
        indent() << type_name(type) << " " << name << ";" << endl;
    }
    indent_down();
    f_consts <<
    "};" << endl;
    
    f_consts_impl <<
    "const " << program_name_ << "Constants g_" << program_name_ << "_constants;" << endl <<
    endl <<
    program_name_ << "Constants::" << program_name_ << "Constants() {" << endl;
    indent_up();
    for (c_iter = consts.begin(); c_iter != consts.end(); ++c_iter) {
        print_const_value(f_consts_impl,
            (*c_iter)->get_name(),
            (*c_iter)->get_type(),
            (*c_iter)->get_value());
    }
    indent_down();
    indent(f_consts_impl) <<
    "}" << endl;
    
    f_consts <<
    endl <<
    "extern const " << program_name_ << "Constants g_" << program_name_ << "_constants;" << endl <<
    endl <<
    ns_close_ << endl <<
    endl <<
    "#endif" << endl;
    f_consts.close();
    
    f_consts_impl <<
    endl <<
    ns_close_ << endl <<
    endl;
}

/**
* Prints the value of a constant with the given type. Note that type checking
* is NOT performed in this function as it is always run beforehand using the
* validate_types method in main.cc
*/
void t_cppemb_generator::print_const_value(ofstream& out, string name, t_type* type, t_const_value* value) {
    type = get_true_type(type);
    if (type->is_base_type()) {
        string v2 = render_const_value(out, name, type, value);
        indent(out) << name << " = " << v2 << ";" << endl <<
        endl;
    } else if (type->is_enum()) {
        indent(out) << name << " = (" << type_name(type) << ")" << value->get_integer() << ";" << endl <<
        endl;
    } else if (type->is_struct() || type->is_xception()) {
        if( value )
        {
            const vector<t_field*>& fields = ((t_struct*)type)->get_members();
            vector<t_field*>::const_iterator f_iter;
            const map<t_const_value*, t_const_value*>& val = value->get_map();
            map<t_const_value*, t_const_value*>::const_iterator v_iter;
            bool is_nonrequired_field = false;
            for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
                t_type* field_type = NULL;
                is_nonrequired_field = false;
                for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
                    if ((*f_iter)->get_name() == v_iter->first->get_string()) {
                        field_type = (*f_iter)->get_type();
                        is_nonrequired_field = (*f_iter)->get_req() != t_field::T_REQUIRED;
                    }
                }
                if (field_type == NULL) {
                    throw "type error: " + type->get_name() + " has no field " + v_iter->first->get_string();
                }
                string val = render_const_value(out, name, field_type, v_iter->second);
                indent(out) << name << "." << v_iter->first->get_string() << " = " << val << ";" << endl;
            }
            out << endl;
        }
    } else if (type->is_map()) {
        throw std::string("type problem: maps not implemented");
        /*
        t_type* ktype = ((t_map*)type)->get_key_type();
        t_type* vtype = ((t_map*)type)->get_val_type();
        const map<t_const_value*, t_const_value*>& val = value->get_map();
        map<t_const_value*, t_const_value*>::const_iterator v_iter;
        for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
        string key = render_const_value(out, name, ktype, v_iter->first);
        string val = render_const_value(out, name, vtype, v_iter->second);
        indent(out) << name << ".insert(std::make_pair(" << key << ", " << val << "));" << endl;
        }
        out << endl;
        */
    } else if (type->is_list()) {
        throw std::string("type problem: lists not implemented");
        /*
        t_type* etype = ((t_list*)type)->get_elem_type();
        const vector<t_const_value*>& val = value->get_list();
        vector<t_const_value*>::const_iterator v_iter;
        for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
        string val = render_const_value(out, name, etype, *v_iter);
        indent(out) << name << ".push_back(" << val << ");" << endl;
        }
        out << endl;
        */
    } else if (type->is_set()) {
        throw std::string("type problem: sets not implemented");
        /*
        t_type* etype = ((t_set*)type)->get_elem_type();
        const vector<t_const_value*>& val = value->get_list();
        vector<t_const_value*>::const_iterator v_iter;
        for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
        string val = render_const_value(out, name, etype, *v_iter);
        indent(out) << name << ".insert(" << val << ");" << endl;
        }
        out << endl;
        */
    } else {
        throw "INVALID TYPE IN print_const_value: " + type->get_name();
    }
}

/**
*
*/
string t_cppemb_generator::render_const_value(ofstream& out, string name, t_type* type, t_const_value* value) {
    (void) name;
    std::ostringstream render;
    
    if (type->is_base_type()) {
        t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
        switch (tbase) {
        case t_base_type::TYPE_STRING:
            if( value ) 
                render << "LSL(\"" << get_escaped_string(value) << "\")";
            else
                render << "ConstBufferReference::empty";
            break;
        case t_base_type::TYPE_BOOL:
            if( value )
                render << ((value->get_integer() > 0) ? "true" : "false");
            else
                render << "false";
            break;
        case t_base_type::TYPE_BYTE:
        case t_base_type::TYPE_I16:
        case t_base_type::TYPE_I32:
            if( value )
                render << value->get_integer();
            else
                render << "0";
            break;
        case t_base_type::TYPE_I64:
            render << value->get_integer() << "LL";
            break;
        case t_base_type::TYPE_DOUBLE:
            if( value )
            {
                if (value->get_type() == t_const_value::CV_INTEGER) {
                    render << value->get_integer();
                } else {
                    render << value->get_double();
                }
            }
            else
                render << "0";
            break;
        default:
            throw "compiler error: no const of base type " + t_base_type::t_base_name(tbase);
        }
    } else if (type->is_enum()) {
        if(value)
            render << "(" << type_name(type) << ")" << value->get_integer();
    } else {
        if( value )
        {
            string t = tmp("tmp");
            indent(out) << type_name(type) << " " << t << ";" << endl;
            print_const_value(out, t, type, value);
            render << t;
        }
    }
    
    return render.str();
}

void t_cppemb_generator::generate_forward_declaration(t_struct* tstruct) {
    // Forward declare struct def
    f_types_ <<
    indent() << "struct " << tstruct->get_name() << ";" << endl <<
    endl;
}

/**
* Generates a struct definition for a thrift data type. This is a class
* with data members and a read/write() function
*
* @param tstruct The struct definition
*/
void t_cppemb_generator::generate_cpp_struct(t_struct* tstruct, bool is_exception) {
    generate_struct_declaration(f_types_, tstruct, is_exception, true, true);
    std::ofstream& out = (f_types_impl_);
    generate_struct_reader(out, tstruct);
    generate_struct_writer(out, tstruct);
}

/**
* Writes the struct declaration into the header file
*
* @param out Output stream
* @param tstruct The struct
*/
void t_cppemb_generator::generate_struct_declaration(ofstream& out,
    t_struct* tstruct,
    bool is_exception,
    bool read,
    bool write
    )
{
    string extends = "";
    if (is_exception) {
        extends = " : public ::apache::thrift::TException";
    }
    
    // Get members
    vector<t_field*>::const_iterator m_iter;
    const vector<t_field*>& members = tstruct->get_members();
    
    out << endl;
    
    // Open struct def
    out <<
    indent() << "struct " << tstruct->get_name() << extends << " {" << endl <<
    endl;
    indent_up();
    
    // Copy constructor
    indent(out) <<
    tstruct->get_name() << "(const " << tstruct->get_name() << "&) = default;" << endl;
    
    // Assignment Operator
    indent(out) << tstruct->get_name() << "& operator=(const " << tstruct->get_name() << "&) = default;" << endl;
    
    // Default constructor
    indent(out) <<
    tstruct->get_name() << "()" << endl;
    
    
    bool init_ctor = false;
    
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
        t_type* t = get_true_type((*m_iter)->get_type());
        if (t->is_base_type() || t->is_enum() )
        {
            string dval;
            indent(out);
            if (t->is_enum())
            {
                dval += "(" + type_name(t) + ")";
            }
            dval += (t->is_string() ) ? "" : "0";
            t_const_value* cv = (*m_iter)->get_value();
            dval = render_const_value(out, (*m_iter)->get_name(), t, cv);
            if (!init_ctor)
            {
                init_ctor = true;
                out << ": ";
                out << (*m_iter)->get_name() << "(" << dval << ")";
            }
            else
            {
                out << ", " << (*m_iter)->get_name() << "(" << dval << ")";
            }
            out << endl;
        }
    }
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter)
    {
        if ((*m_iter)->get_req() != t_field::T_REQUIRED)
            indent(out) << ", isSet_" << (*m_iter)->get_name() << "(false)" << endl;
    }
    indent(out) << "{" << endl;
    indent_up();
    // TODO(dreiss): When everything else in Thrift is perfect,
    // do more of these in the initializer list.
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
        t_type* t = get_true_type((*m_iter)->get_type());
        
        if (!t->is_base_type()) {
            t_const_value* cv = (*m_iter)->get_value();
            if (cv != NULL) {
                print_const_value(out, (*m_iter)->get_name(), t, cv);
            }
        }
    }
    scope_down(out);
    
    out << endl;
    
    // Declare all fields
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter)
    {
        indent(out) <<
        declare_field(*m_iter) << endl;
    }

    out << endl;

    // Declare all optionals
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter)
    {
        if ((*m_iter)->get_req() != t_field::T_REQUIRED)
            indent(out) << "bool isSet_" << (*m_iter)->get_name() << ":1;" << endl;
    }
    
    out << endl;
    if (read) {
        out <<
        indent() << "bool read(::Thrift::Reader& reader);" << endl;
    }
    if (write) {
        out <<
        indent() << "bool write(::Thrift::Writer& writer) const;" << endl;
    }
    out << endl;
    
    indent_down();
    indent(out) <<
    "};" << endl <<
    endl;
    
}


/**
* Makes a helper function to gen a struct reader.
*
* @param out Stream to write to
* @param tstruct The struct
*/
void t_cppemb_generator::generate_struct_reader_fragment(ofstream& out,
    t_struct* tstruct)
{
    indent(out) << "bool readResult = true;" << endl;
    
    const vector<t_field*>& fields = tstruct->get_members();
    vector<t_field*>::const_iterator f_iter;
    
    // generate bools to check whether required fields are present
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
    {
        bool check_if_req = (*f_iter)->get_req() == t_field::T_REQUIRED;
        if (check_if_req)
        {
            out << indent() << "bool " << (*f_iter)->get_name() << "Present = false;" << endl;
        }
        else
            out << indent() << "isSet_" << (*f_iter)->get_name() << " = false;" << endl;
    }
    
    
    out <<
    indent() << "while( reader.parseNextField() && readResult )" << endl <<
    indent() << "{" << endl;
    indent_up();
    out <<
    indent() << "const ::Thrift::Reader::Field& field = reader.getParsedField();" << endl <<
    indent() << "if( field.type == ::Thrift::FieldType::STOP )" << endl;
    indent_up(); out << indent() << "break;" << endl; indent_down();
    out <<
    indent() << "switch( field.id )" << endl;
    indent(out) << "{" << endl;
    // Generate deserialization code for known cases
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
    {
        indent(out) <<
        "case " << (*f_iter)->get_key() << ":" << endl;
        indent_up();
        indent(out) <<
        "if( field.type == " << type_to_enum((*f_iter)->get_type()) << ")" << endl;
        indent(out) <<"{" << endl; indent_up();
        
        generate_deserialize_field(out, *f_iter);
        if( (*f_iter)->get_req() == t_field::T_REQUIRED )
        {
            out << indent() << (*f_iter)->get_name() << "Present = true;" << endl;
        }
        else
            out << indent() << "isSet_" << (*f_iter)->get_name() << " = true;" << endl;
            
        indent_down();
        indent(out) << "}" << endl;
        indent(out) << "else" << endl;
        indent_up();
        indent(out) << "readResult = false;" << endl;
        indent_down();
        indent(out) << "break;" << endl;
        indent_down();
    }
    indent(out) << "}" << endl;
    indent_down();
    indent(out) << "}" << endl << endl;
    // check for presence of required fields
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
    {
        bool check_if_req = (*f_iter)->get_req() == t_field::T_REQUIRED;
        if (check_if_req)
        {
            out << indent() << "if( ! " << (*f_iter)->get_name() << "Present )" << endl;
            indent_up();
            indent(out) << "readResult = false;" << endl;
            indent_down();
        }
    }
}

void t_cppemb_generator::generate_struct_reader(ofstream& out,
    t_struct* tstruct)
{
    indent(out) << "bool " << tstruct->get_name() <<
    "::read(::Thrift::Reader& reader)" << endl << "{" << endl;
    indent_up();
    generate_struct_reader_fragment(out,tstruct);
    indent(out) << "return readResult;" << endl;
    indent_down();
    indent(out) << "}" << endl << endl;
}

/**
* Generates the write function.
*
* @param out Stream to write to
* @param tstruct The struct
*/
void t_cppemb_generator::generate_struct_writer(ofstream& out,
    t_struct* tstruct)
{
    string name = tstruct->get_name();
    const vector<t_field*>& fields = tstruct->get_sorted_members();
    vector<t_field*>::const_iterator f_iter;
    
    indent(out) <<
    "bool " << tstruct->get_name() <<
    "::write(::Thrift::Writer &writer) const" << endl << "{" << endl;
    indent_up();
    
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
    {
        generate_serialize_field(out, *f_iter );
    }
    // Write field closer
    out << endl << indent() << "writer.addStop();" << endl << endl;
    indent(out) << "return ! writer.hasErrors();" << endl;
    indent_down();
    indent(out) << "}" << endl << endl;
}

/**
* Struct writer for result of a function, which can have only one of its
* fields set and does a conditional if else look up into the __isset field
* of the struct.
*
* @param out Output stream
* @param tstruct The result struct
*/
void t_cppemb_generator::generate_struct_result_writer(ofstream& out,
    t_struct* tstruct)
{
    string name = tstruct->get_name();
    const vector<t_field*>& fields = tstruct->get_sorted_members();
    vector<t_field*>::const_iterator f_iter;
    
    indent(out) <<
    "bool " << tstruct->get_name() <<
    "::write(::Thrift::Writer& writer) const" << endl << "{" << endl;
    indent_up();
    
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
    {
        generate_serialize_field(out, *f_iter );
    }
    
    out << endl << indent() << "return ! writer.hasErrors();" << endl;
    
    indent_down();
    indent(out) << "}" << endl << endl;
}

namespace struct_ostream_operator_generator
{
    static void generate_required_field_value(std::ofstream& out, const t_field* field)
    {
        out << " << to_string(obj." << field->get_name() << ")";
    }
    
    static void generate_optional_field_value(std::ofstream& out, const t_field* field)
    {
        out << "; (obj.__isset." << field->get_name() << " ? (out";
        generate_required_field_value(out, field);
        out << ") : (out << \"<null>\"))";
    }
    
    static void generate_field_value(std::ofstream& out, const t_field* field)
    {
        if (field->get_req() == t_field::T_OPTIONAL)
            generate_optional_field_value(out, field);
        else
            generate_required_field_value(out, field);
    }
    
    static void generate_field_name(std::ofstream& out, const t_field* field)
    {
        out << "\"" << field->get_name() << "=\"";
    }
    
    static void generate_field(std::ofstream& out, const t_field* field)
    {
        generate_field_name(out, field);
        generate_field_value(out, field);
    }
    
    static void generate_fields(std::ofstream& out,
        const vector<t_field*>& fields,
        const std::string& indent)
    {
        const vector<t_field*>::const_iterator beg = fields.begin();
        const vector<t_field*>::const_iterator end = fields.end();
        
        for (vector<t_field*>::const_iterator it = beg; it != end; ++it) {
            out << indent << "out << ";
            
            if (it != beg) {
                out << "\", \" << ";
            }
            
            generate_field(out, *it);
            out << ";" << endl;
        }
    }
}


/**
* Generates a thrift service. In C++, this comprises an entirely separate
* header and source file. The header file defines the methods and includes
* the data types defined in the main header file, and the implementation
* file contains implementations of the basic printer and default interfaces.
*
* @param tservice The service definition
*/
void t_cppemb_generator::generate_service(t_service* tservice) {
    string svcname = tservice->get_name();
    
    // Make output files
    string f_header_name = get_out_dir()+svcname+"_embedded.h";
    f_header_.open(f_header_name.c_str());
    
    // Print header file includes
    f_header_ <<
    autogen_comment();
    f_header_ <<
    "#ifndef " << svcname << "_EMBEDDED_H" << endl <<
    "#define " << svcname << "_EMBEDDED_H" << endl <<
    endl;
    // FIXME: header path
    f_header_ << "#include \"MfiAction.hxx\"" << endl;
    f_header_ <<
    "#include \"" << get_include_prefix(*get_program()) << program_name_ <<
    "_embedded_types.h\"" << endl;
    
    t_service* extends_service = tservice->get_extends();
    if (extends_service != NULL) {
        f_header_ <<
        "#include \"" << get_include_prefix(*(extends_service->get_program())) <<
        extends_service->get_name() << "_embedded.h\"" << endl;
    }
    
    f_header_ <<
    endl <<
    ns_open_ << endl <<
    endl;
    
    // Service implementation file includes
    string f_service_name = get_out_dir()+svcname+"_embedded.cpp";
    f_service_.open(f_service_name.c_str());
    f_service_ <<
    autogen_comment();
    f_service_ <<
    "#include \"" << get_include_prefix(*get_program()) << svcname << "_embedded.h\"" << endl;
    f_service_ <<
    // FIXME: header path
    "#include \"ThriftWriter.hxx\"" << endl <<
    "#include \"ThriftReader.hxx\"" << endl <<
    "#include \"ThriftDispatcher.hxx\"" << endl;
    
    f_service_ <<
    endl << ns_open_ << endl << endl;
    
    // Generate all the components
    f_header_ << "namespace " << service_name_ << endl << "{" << endl;
    generate_service_interface(tservice);
    generate_service_client(tservice);
    f_header_ << "} // namespace " << service_name_ << endl;
    generate_service_implementation(tservice);
    generate_service_client_implementation(tservice);
//    generate_service_interface_factory(tservice);
//    generate_service_null(tservice);
    generate_service_skeleton(tservice);
    
    
    // Close the namespace
    f_service_ <<
    ns_close_ << endl <<
    endl;
    f_header_ <<
    ns_close_ << endl <<
    endl;
    
    f_header_ <<
    "#endif" << endl;
    
    // Close the files
    f_service_.close();
    f_header_.close();
}

/**
* Generates a service interface definition.
*
* @param tservice The service to generate a header definition for
*/
void t_cppemb_generator::generate_service_interface(t_service* tservice)
{
    vector<t_function*> functions = tservice->get_functions();
    vector<t_function*>::iterator f_iter;
    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter)
    {
        if ((*f_iter)->has_doc()) f_header_ << endl;
            generate_java_doc(f_header_, *f_iter);
        f_header_ <<  "class " << (*f_iter)->get_name() << "Action : private ::Mfi::Action" << endl << "{" << endl;
        indent_up();
        indent(f_header_) << "virtual void execute() override;" << endl;
        indent(f_header_) << "virtual const ConstBufferReference& getName() const noexcept override;" << endl;
        indent(f_header_) << (*f_iter)->get_name() << "Action( ::Mfi::ActionParserParameters* p = nullptr) : Mfi::Action(p) {}" << endl;
        indent(f_header_) << "virtual ~" << (*f_iter)->get_name() << "Action();" << endl;
        f_header_ << "public:" << endl;
        indent(f_header_) << "static ::Mfi::Action *create( ::Mfi::ActionParserParameters *);" << endl;
        indent(f_header_) << "static const ConstBufferReference actionName;" << endl;
        

        t_struct *arglist = (*f_iter)->get_arglist();
        const t_struct::members_type& args = arglist->get_members();
        t_type *returnType = (*f_iter)->get_returntype();
        
        f_header_ << endl;
        indent(f_header_) << "// function to be implemented by user" << endl;
        indent(f_header_) << type_name(returnType) << " " << (*f_iter)->get_name() << "(";
        f_header_ << argument_list(arglist, true, false, true) << ");" << endl;

        indent_down();
        f_header_ << "};" << endl << endl;
    }
}

/**
* Generates a service interface implementation.
*
* @param tservice The service to generate a source implementation for
*/
void t_cppemb_generator::generate_service_implementation(t_service* tservice)
{
    std::ofstream& out = f_service_;
    vector<t_function*> functions = tservice->get_functions();
    vector<t_function*>::iterator f_iter;
    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter)
    {
        t_struct *arglist = (*f_iter)->get_arglist();
        const t_struct::members_type& args = arglist->get_members();
        t_type *returnType = (*f_iter)->get_returntype();

        out << "/*" << endl;
        out << " * " << (*f_iter)->get_name() << "Action implementation" << endl;
        out << " */" << endl;
        out << "const ConstBufferReference " << service_name_ << "::" << (*f_iter)->get_name() << "Action::actionName = { LSL(\"" << (*f_iter)->get_name() << "\") };" << endl << endl;

        out << service_name_ << "::" << (*f_iter)->get_name() << "Action::~"<< (*f_iter)->get_name() << "Action()" << endl << "{" << endl << "}" << endl << endl;
        out << "const ConstBufferReference &" << service_name_ << "::" << (*f_iter)->get_name() << "Action::getName() const noexcept" << endl;
        out << "{" << endl;
        indent_up();
        indent(out) << "return " << service_name_ << "::" << (*f_iter)->get_name() << "Action::actionName;" << endl;
        indent_down();
        out << "}" << endl << endl;
        
        out << "::Mfi::Action *" << service_name_ << "::" << (*f_iter)->get_name() << "Action::create( ::Mfi::ActionParserParameters *p)" << endl;
        out << "{" << endl;
        indent_up();
        indent(out) << "return new " << service_name_ << "::" << (*f_iter)->get_name() << "Action(p);" << endl;
        indent_down();
        out << "}" << endl << endl;

        out << "void " << service_name_ << "::" << (*f_iter)->get_name() << "Action::execute()" << endl;
        out << "{" << endl;
        indent_up();
        indent(out) << "::Thrift::Dispatcher &dispatcher(::Thrift::Dispatcher::getInstance());" << endl;
        if( ! args.empty() )
        {
            for( t_struct::members_type::const_iterator m_iter = args.begin(); m_iter != args.end(); ++m_iter )
            {
                indent(out) << type_name((*m_iter)->get_type()) << " " << (*m_iter)->get_name() << ";" << endl;
                if( (*m_iter)->get_req() != t_field::T_REQUIRED )
                    indent(out) << "bool isSet_" << (*m_iter)->get_name() << " = false;" << endl;
            }
            indent(out) << "bool readResult = true;" << endl;
            indent(out) << "while( parameters->reader->parseNextField() && readResult )" << endl;
            indent(out) << "{" << endl;
            indent_up();
            indent(out) << "const ::Thrift::Reader::Field& field = parameters->reader->getParsedField();" << endl;
            indent(out) << "if( field.type == ::Thrift::FieldType::STOP )" << endl;
            indent_up(); indent(out) << "break;" << endl; indent_down();
            indent(out) << "switch( field.id )" << endl;
            indent(out) << "{" << endl;
            // Generate deserialization code for known cases
            for (t_struct::members_type::const_iterator m_iter = args.begin(); m_iter != args.end(); ++m_iter)
            {
                indent(out) << "case " << (*m_iter)->get_key() << ":" << endl;
                indent_up();
                indent(out) << "if( field.type == " << type_to_enum((*m_iter)->get_type()) << ")" << endl;
                indent(out) <<"{" << endl; indent_up();
                generate_deserialize_field(out, *m_iter);
                if( (*m_iter)->get_req() != t_field::T_REQUIRED )
                    indent(out) << "isSet_" << (*m_iter)->get_name() << " = true;" << endl;
                indent_down();
                indent(out) << "}" << endl;
                indent(out) << "else" << endl;
                indent_up();
                indent(out) << "readResult = false;" << endl;
                indent_down();
                indent(out) << "break;" << endl;
                indent_down();
            }
            indent(out) << "}" << endl;
            indent_down();
            indent(out) << "}" << endl;
            indent(out) << "if( ! readResult )" << endl;
            indent(out) << "{" << endl;
            indent_up();
            indent(out) << "dispatcher.sendException(actionName, LSL(\"invalid parameters\"), parameters->sequenceNumber, parameters->referenceUID );" << endl;
            indent(out) << "return;" << endl;
            indent_down();
            indent(out) << "}" << endl;
        }
        
        out << endl;

        indent(out) << "// call user implementation" << endl;
        indent(out);
        if( ! returnType->is_void() )
            out << type_name(returnType) << " result = ";
        out << (*f_iter)->get_name() << "(" << argument_list(arglist, true, false, true, false) << ");" << endl;

        
        out << endl;
        // generate writer for answer
        indent(out) << "::Thrift::Writer* writerP = dispatcher.startAnswer(actionName, Thrift::MessageType::REPLY, parameters->sequenceNumber);" << endl;
        indent(out) << "if( writerP )" << endl;
        indent(out) << "{" << endl;
        indent_up();
        if( ! returnType->is_void() )
        {
            indent(out) << "::Thrift::Writer& writer = *writerP;" << endl << endl;
            // returns are always encapsulated in a struct
            indent(out) << "writer.openStruct(0);" << endl;
            generate_serialize_type( out, returnType, "result", 0 );
            indent(out) << "writer.closeStruct();" << endl;
            out << endl;
        }
        indent(out) << "writerP->addStop();" << endl;
        indent(out) << "dispatcher.finishAnswer(writerP,parameters->referenceUID);" << endl;
        indent_down();
        indent(out) << "}" << endl;

        
        indent_down();
        out << "}" << endl <<
        endl;
    }
}

/**
* Generates a service interface factory.
*
* @param tservice The service to generate an interface factory for.
*/
void t_cppemb_generator::generate_service_interface_factory(t_service* tservice)
{
    f_header_ << "// TODO: implement service interface factory" << endl;
    #if 0
    string service_if_name = service_name_ + style + "If";
    
    // Figure out the name of the upper-most parent class.
    // Getting everything to work out properly with inheritance is annoying.
    // Here's what we're doing for now:
    //
    // - All handlers implement getHandler(), but subclasses use covariant return
    //   types to return their specific service interface class type.  We have to
    //   use raw pointers because of this; shared_ptr<> can't be used for
    //   covariant return types.
    //
    // - Since we're not using shared_ptr<>, we also provide a releaseHandler()
    //   function that must be called to release a pointer to a handler obtained
    //   via getHandler().
    //
    //   releaseHandler() always accepts a pointer to the upper-most parent class
    //   type.  This is necessary since the parent versions of releaseHandler()
    //   may accept any of the parent types, not just the most specific subclass
    //   type.  Implementations can use dynamic_cast to cast the pointer to the
    //   subclass type if desired.
    t_service* base_service = tservice;
    while (base_service->get_extends() != NULL) {
        base_service = base_service->get_extends();
    }
    string base_if_name = type_name(base_service) + style + "If";
    
    // Generate the abstract factory class
    string factory_name = service_if_name + "Factory";
    string extends;
    if (tservice->get_extends() != NULL) {
        extends = " : virtual public " + type_name(tservice->get_extends()) +
        style + "IfFactory";
    }
    
    f_header_ <<
    "class " << factory_name << extends << " {" << endl <<
    " public:" << endl;
    indent_up();
    f_header_ <<
    indent() << "typedef " << service_if_name << " Handler;" << endl <<
    endl <<
    indent() << "virtual ~" << factory_name << "() {}" << endl <<
    endl <<
    indent() << "virtual " << service_if_name << "* getHandler(" <<
    "const ::apache::thrift::TConnectionInfo& connInfo) = 0;" <<
    endl <<
    indent() << "virtual void releaseHandler(" << base_if_name <<
    "* /* handler */) = 0;" << endl;
    
    indent_down();
    f_header_ <<
    "};" << endl << endl;
    
    // Generate the singleton factory class
    string singleton_factory_name = service_if_name + "SingletonFactory";
    f_header_ <<
    "class " << singleton_factory_name <<
    " : virtual public " << factory_name << " {" << endl <<
    " public:" << endl;
    indent_up();
    f_header_ <<
    indent() << singleton_factory_name << "(const boost::shared_ptr<" <<
    service_if_name << ">& iface) : iface_(iface) {}" << endl <<
    indent() << "virtual ~" << singleton_factory_name << "() {}" << endl <<
    endl <<
    indent() << "virtual " << service_if_name << "* getHandler(" <<
    "const ::apache::thrift::TConnectionInfo&) {" << endl <<
    indent() << "  return iface_.get();" << endl <<
    indent() << "}" << endl <<
    indent() << "virtual void releaseHandler(" << base_if_name <<
    "* /* handler */) {}" << endl;
    
    f_header_ <<
    endl <<
    " protected:" << endl <<
    indent() << "boost::shared_ptr<" << service_if_name << "> iface_;" << endl;
    
    indent_down();
    f_header_ <<
    "};" << endl << endl;
    #endif
}

/**
* Generates a null implementation of the service.
*
* @param tservice The service to generate a header definition for
*/
void t_cppemb_generator::generate_service_null(t_service* tservice)
{
    f_header_ << "// TODO: implement null service" << endl;
    #if 0
    string extends = "";
    if (tservice->get_extends() != NULL) {
        extends = " , virtual public " + type_name(tservice->get_extends()) + style + "Null";
    }
    f_header_ <<
    "class " << service_name_ << style << "Null : virtual public " << service_name_ << style << "If" << extends << " {" << endl <<
    " public:" << endl;
    indent_up();
    f_header_ <<
    indent() << "virtual ~" << service_name_ << style << "Null() {}" << endl;
    vector<t_function*> functions = tservice->get_functions();
    vector<t_function*>::iterator f_iter;
    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
        f_header_ <<
        indent() << function_signature(*f_iter, style, "", false) << " {" << endl;
        indent_up();
        
        t_type* returntype = (*f_iter)->get_returntype();
        t_field returnfield(returntype, "_return");
        
        if (style == "") {
            if (returntype->is_void() || is_complex_type(returntype)) {
                f_header_ << indent() << "return;" << endl;
            } else {
                f_header_ <<
                indent() << declare_field(&returnfield, true) << endl <<
                indent() << "return _return;" << endl;
            }
        } else if (style == "CobSv") {
            if (returntype->is_void()) {
                f_header_ << indent() << "return cob();" << endl;
            } else {
                t_field returnfield(returntype, "_return");
                f_header_ <<
                indent() << declare_field(&returnfield, true) << endl <<
                indent() << "return cob(_return);" << endl;
            }
            
        } else {
            throw "UNKNOWN STYLE";
        }
        
        indent_down();
        f_header_ <<
        indent() << "}" << endl;
    }
    indent_down();
    f_header_ <<
    "};" << endl << endl;
    
    #endif
}

void t_cppemb_generator::generate_function_call(ostream& out, t_function* tfunction, string target, string iface, string arg_prefix) {
    
    out << "// TODO: implement function call" << endl;
    #if 0
    bool first = true;
    t_type* ret_type = get_true_type(tfunction->get_returntype());
    out << indent();
    if (!tfunction->is_oneway() && !ret_type->is_void()) {
        if (is_complex_type(ret_type)) {
            first = false;
            out << iface << "->" << tfunction->get_name() << "(" << target;
        } else {
            out << target << " = " << iface << "->" << tfunction->get_name() << "(";
        }
    } else {
        out << iface << "->" << tfunction->get_name() << "(";
    }
    const std::vector<t_field*>& fields = tfunction->get_arglist()->get_members();
    vector<t_field*>::const_iterator f_iter;
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
        if (first) {
            first = false;
        } else {
            out << ", ";
        }
        out << arg_prefix << (*f_iter)->get_name();
    }
    out << ");" << endl;
    #endif
}


#if 0
/**
* Generates a multiface, which is a single server that just takes a set
* of objects implementing the interface and calls them all, returning the
* value of the last one to be called.
*
* @param tservice The service to generate a multiserver for.
*/
void t_cppemb_generator::generate_service_multiface(t_service* tservice) {
    // Generate the dispatch methods
    vector<t_function*> functions = tservice->get_functions();
    vector<t_function*>::iterator f_iter;
    
    string extends = "";
    string extends_multiface = "";
    if (tservice->get_extends() != NULL) {
        extends = type_name(tservice->get_extends());
        extends_multiface = ", public " + extends + "Multiface";
    }
    
    string list_type = string("std::vector<boost::shared_ptr<") + service_name_ + "If> >";
    
    // Generate the header portion
    f_header_ <<
    "class " << service_name_ << "Multiface : " <<
    "virtual public " << service_name_ << "If" <<
    extends_multiface << " {" << endl <<
    " public:" << endl;
    indent_up();
    f_header_ <<
    indent() << service_name_ << "Multiface(" << list_type << "& ifaces) : ifaces_(ifaces) {" << endl;
    if (!extends.empty()) {
        f_header_ <<
        indent() << "  std::vector<boost::shared_ptr<" + service_name_ + "If> >::iterator iter;" << endl <<
        indent() << "  for (iter = ifaces.begin(); iter != ifaces.end(); ++iter) {" << endl <<
        indent() << "    " << extends << "Multiface::add(*iter);" << endl <<
        indent() << "  }" << endl;
    }
    f_header_ <<
    indent() << "}" << endl <<
    indent() << "virtual ~" << service_name_ << "Multiface() {}" << endl;
    indent_down();
    
    // Protected data members
    f_header_ <<
    " protected:" << endl;
    indent_up();
    f_header_ <<
    indent() << list_type << " ifaces_;" << endl <<
    indent() << service_name_ << "Multiface() {}" << endl <<
    indent() << "void add(boost::shared_ptr<" << service_name_ << "If> iface) {" << endl;
    if (!extends.empty()) {
        f_header_ <<
        indent() << "  " << extends << "Multiface::add(iface);" << endl;
    }
    f_header_ <<
    indent() << "  ifaces_.push_back(iface);" << endl <<
    indent() << "}" << endl;
    indent_down();
    
    f_header_ <<
    indent() << " public:" << endl;
    indent_up();
    
    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
        t_struct* arglist = (*f_iter)->get_arglist();
        const vector<t_field*>& args = arglist->get_members();
        vector<t_field*>::const_iterator a_iter;
        
        string call = string("ifaces_[i]->") + (*f_iter)->get_name() + "(";
        bool first = true;
        if (is_complex_type((*f_iter)->get_returntype())) {
            call += "_return";
            first = false;
        }
        for (a_iter = args.begin(); a_iter != args.end(); ++a_iter) {
            if (first) {
                first = false;
            } else {
                call += ", ";
            }
            call += (*a_iter)->get_name();
        }
        call += ")";
        
        f_header_ <<
        indent() << function_signature(*f_iter, "") << " {" << endl;
        indent_up();
        f_header_ <<
        indent() << "size_t sz = ifaces_.size();" << endl <<
        indent() << "size_t i = 0;" << endl <<
        indent() << "for (; i < (sz - 1); ++i) {" << endl;
        indent_up();
        f_header_ <<
        indent() << call << ";" << endl;
        indent_down();
        f_header_ <<
        indent() << "}" << endl;
        
        if (!(*f_iter)->get_returntype()->is_void()) {
            if (is_complex_type((*f_iter)->get_returntype())) {
                f_header_ <<
                indent() << call << ";" << endl <<
                indent() << "return;" << endl;
            } else {
                f_header_ <<
                indent() << "return " << call << ";" << endl;
            }
        } else {
            f_header_ <<
            indent() << call << ";" << endl;
        }
        
        indent_down();
        f_header_ <<
        indent() << "}" << endl <<
        endl;
    }
    
    indent_down();
    f_header_ <<
    indent() << "};" << endl <<
    endl;
}

#endif

/**
* Generates a service client definition.
*
* @param tservice The service to generate a server for.
*/
void t_cppemb_generator::generate_service_client(t_service* tservice) {
    
    vector<t_function*> functions = tservice->get_functions();
    vector<t_function*>::iterator f_iter;
    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter)
    {
        if ((*f_iter)->has_doc())
            f_header_ << endl;
        generate_java_doc(f_header_, *f_iter);
        t_struct *arglist = (*f_iter)->get_arglist();
        t_type *returnType = (*f_iter)->get_returntype();
            
        f_header_ <<  "class " << (*f_iter)->get_name() << "ClientAction : private ::Mfi::ClientAction" << endl << "{" << endl;
        indent_up();
        indent(f_header_) << "virtual const ConstBufferReference& getName() const noexcept override;" << endl;
        indent(f_header_) << "virtual void callAnswered( ::Mfi::ActionParserParameters *parameters ) override;" << endl;
        f_header_ << "public:" << endl;
        indent(f_header_) << "virtual ~" << (*f_iter)->get_name() << "ClientAction();" << endl << endl;
        
        indent(f_header_) << "class Callback : public ::Mfi::ClientAction::Callback" << endl << indent() << "{" << endl;
        indent(f_header_) << "public:" << endl; 
        indent_up();
        indent(f_header_) << "Callback() = default;" << endl;
        indent(f_header_) << "Callback(const Callback&) = default;" << endl;
        indent(f_header_) << "Callback& operator=(const Callback&) = default;" << endl << endl;
        indent(f_header_) << "virtual void " << (*f_iter)->get_name() << "Result(";
        if( ! returnType->is_void()  )
            f_header_ << type_name(returnType, false, true);
        f_header_ << ") = 0;" << endl;
        indent_down();
        indent(f_header_) << "};" << endl << endl;
        indent(f_header_) << "static void " << (*f_iter)->get_name() << "(" << argument_list((*f_iter)->get_arglist(), true);
        if( ! arglist->get_members().empty() )
            f_header_ << ", ";
        f_header_ << "Callback *resultCallback";
        const t_struct::members_type args = arglist->get_members();
        for (t_struct::members_type::const_iterator m_iter = args.begin(); m_iter != args.end(); ++m_iter)
        {
            if( (*m_iter)->get_req() != t_field::T_REQUIRED )
                f_header_ << ", bool isSet_" << (*m_iter)->get_name() << " = true";
        }
        
        f_header_ << ");" << endl;
        indent_down();
        f_header_ << "};" << endl << endl;
    }
}

/**
* Generates a service client definition.
*
* @param tservice The service to generate a server for.
*/
void t_cppemb_generator::generate_service_client_implementation(t_service* tservice) {
    
    vector<t_function*> functions = tservice->get_functions();
    vector<t_function*>::iterator f_iter;
    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter)
    {
        f_service_ << "/*" << endl;
        f_service_ << " * " << (*f_iter)->get_name() << "ClientAction implementation" << endl;
        f_service_ << " */" << endl << endl;
        f_service_ << service_name_ << "::" << (*f_iter)->get_name() << "ClientAction::~" << (*f_iter)->get_name() << "ClientAction()" << endl;
        f_service_ << "{" << endl;
        f_service_ << "}" << endl << endl;
        f_service_ << "const ConstBufferReference& " << service_name_ << "::" << (*f_iter)->get_name() << "ClientAction::getName() const noexcept" << endl;
        f_service_ << "{" << endl;
        indent_up();
        indent(f_service_) << "return " << service_name_ << "::" << (*f_iter)->get_name() << "Action::actionName;" << endl;
        indent_down();
        f_service_ << "}" << endl << endl;
        
        
        t_struct *arglist = (*f_iter)->get_arglist();
        t_type *returnType = (*f_iter)->get_returntype();
            
        const t_struct::members_type args = arglist->get_members();
        f_service_ << "void " << service_name_ << "::" << (*f_iter)->get_name() << "ClientAction::" << (*f_iter)->get_name()  << "(" << argument_list(arglist, true);
        if( ! args.empty() )
            f_service_ << ", ";
        f_service_ << "Callback *resultCallback";
        for (t_struct::members_type::const_iterator m_iter = args.begin(); m_iter != args.end(); ++m_iter)
        {
            if( (*m_iter)->get_req() != t_field::T_REQUIRED )
                indent(f_service_) << ", bool isSet_" << (*m_iter)->get_name();
        }

        f_service_ << ")" << endl;
        f_service_ << "{" << endl;
        indent_up();
        indent(f_service_) << service_name_ << "::" << (*f_iter)->get_name() << "ClientAction *action = new " << service_name_ << "::" << (*f_iter)->get_name() << "ClientAction();" << endl;
        indent(f_service_) << "action->callback = resultCallback;" << endl;
        indent(f_service_) << "::Thrift::Dispatcher &dispatcher(::Thrift::Dispatcher::getInstance());" << endl;
        indent(f_service_) << "::Thrift::Writer* writerP = dispatcher.startClientMessage(action);" << endl;
        indent(f_service_) << "if( writerP )" << endl;
        indent(f_service_) << "{" << endl;
        indent_up();
        if( ! args.empty() )
        {
            indent(f_service_) << "::Thrift::Writer& writer = *writerP;" << endl;
            for( t_struct::members_type::const_iterator m_iter = args.begin(); m_iter != args.end(); ++m_iter )
            {
                generate_serialize_field( f_service_, *m_iter );
            }
        }
        f_service_ << endl;
        indent(f_service_) << "dispatcher.finishClientMessage(action, writerP);" << endl;
        indent_down();
        indent(f_service_) << "}" << endl;
        indent(f_service_) << "else" << endl << indent() << "{" << endl;
        indent_up();
        indent(f_service_) << "resultCallback->exceptionResult(::Thrift::TApplicationException());" << endl;
        indent_down();
        indent(f_service_) << "}" << endl;
        indent(f_service_) << "action->release();" << endl;
        
        indent_down();
        f_service_ << "}" << endl << endl;
        
        f_service_ << "void " << service_name_ << "::" << (*f_iter)->get_name() << "ClientAction::callAnswered( ::Mfi::ActionParserParameters *parameters )" << endl;
        f_service_ << "{" << endl;
        indent_up();
        if( ! returnType->is_void() )
        {
            indent(f_service_) << "bool readResult = true;" << endl;
            indent(f_service_) << "::Thrift::Reader &reader = *parameters->reader;" << endl;
            indent(f_service_) << type_name( returnType ) << " " << "result";
            if( returnType->is_string() )
                f_service_ << " = ConstBufferReference::empty;" << endl;
            else if( returnType->is_base_type() )
                f_service_ << " = 0;" << endl;
            else
                f_service_ << ";" << endl;
            indent(f_service_) << "readResult = reader.parseNextField();" << endl;
            indent(f_service_) << "if( readResult )" << endl;
            indent(f_service_) << "{" << endl;
            indent_up();
            if( ! returnType->is_base_type() )
                indent(f_service_) << "readResult = result.read( *parameters->reader );" << endl;
            else
            {
                indent(f_service_) << "const Field& field = reader.getParsedField();" << endl;
                generate_deserialize_type( f_service_, returnType, "result" );
            }
            indent_down();
            indent(f_service_) << "}" << endl;
            f_service_ << endl;
            indent(f_service_) << "if( ! readResult )" << endl;
            indent_up();
            indent(f_service_) << "callback->exceptionResult(::Thrift::TApplicationException());" << endl;
            indent_down();
            indent(f_service_) << "else" << endl;
            indent_up();
        }
        indent(f_service_) << "static_cast<" << service_name_ << "::" << (*f_iter)->get_name() << "ClientAction::Callback*>(callback)->" << (*f_iter)->get_name() << "Result(";
        if( ! returnType->is_void() )
            f_service_ << "result";
        f_service_ << ");" << endl;
        indent_down();
        f_service_ << "}" << endl << endl;
    }
}

/**
* Generates a struct and helpers for a function.
*
* @param tfunction The function
*/
void t_cppemb_generator::generate_function_helpers(t_service* tservice,
    t_function* tfunction)
{
    if (tfunction->is_oneway()) {
        return;
    }
    
    std::ofstream& out = f_service_;
    
    t_struct result(program_, tservice->get_name() + "_" + tfunction->get_name() + "_result");
    t_field success(tfunction->get_returntype(), "success", 0);
    if (!tfunction->get_returntype()->is_void()) {
        result.append(&success);
    }
    
    t_struct* xs = tfunction->get_xceptions();
    const vector<t_field*>& fields = xs->get_members();
    vector<t_field*>::const_iterator f_iter;
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
        result.append(*f_iter);
    }
    
    generate_struct_declaration(f_header_, &result, false);
    generate_struct_reader(out, &result);
    generate_struct_result_writer(out, &result);
    
    result.set_name(tservice->get_name() + "_" + tfunction->get_name() + "_presult");
    generate_struct_declaration(f_header_, &result, false, true, true);
    generate_struct_reader(out, &result);
}


/**
* Generates a skeleton file of a server
*
* @param tservice The service to generate a server for.
*/
void t_cppemb_generator::generate_service_skeleton(t_service* tservice) {
    string svcname = tservice->get_name();
    
    // Service implementation file includes
    string f_skeleton_name = get_out_dir()+svcname+"_server_embedded.skeleton.cpp";
    
    string ns = namespace_prefix(tservice->get_program()->get_namespace("cpp"));
    
    ofstream f_skeleton;
    f_skeleton.open(f_skeleton_name.c_str());
    f_skeleton <<
    "// This autogenerated skeleton file illustrates how to build a server." << endl <<
    "// You should copy it to another filename to avoid overwriting it." << endl <<
    endl <<
    "#include \"" << get_include_prefix(*get_program()) << svcname << "_embedded.h\"" << endl <<
    endl;
    
    // the following code would not compile:
    // using namespace ;
    // using namespace ::;
    if ( (!ns.empty()) && (ns.compare(" ::") != 0)) {
        f_skeleton <<
        "using namespace " << string(ns, 0, ns.size()-2);
        if( use_embedded_namespace_ )
            f_skeleton << "::embedded";
        f_skeleton << ";" << endl <<
        endl;
    }
    
    vector<t_function*> functions = tservice->get_functions();
    vector<t_function*>::iterator f_iter;
    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter)
    {
        t_struct *arglist = (*f_iter)->get_arglist();
        const t_struct::members_type& args = arglist->get_members();
        t_type *returnType = (*f_iter)->get_returntype();

        generate_java_doc(f_skeleton, *f_iter);
        f_skeleton << type_name(returnType) << " " << svcname << "::" << (*f_iter)->get_name() << "Action::" << (*f_iter)->get_name() << "(";
        f_skeleton << argument_list( arglist, true, false, true ) << ")" << endl;
        f_skeleton << "{" << endl;
        indent(f_skeleton) << "// your code here" << endl;
        f_skeleton << "}" << endl << endl;
    }
    
    // Close the files
    f_skeleton.close();
}

/**
* Deserializes a field of any type.
*/

void t_cppemb_generator::generate_deserialize_type(ofstream& out,
    t_type* type,
    string name)
{
    if (type->is_struct() || type->is_xception()) {
        generate_deserialize_struct(out, (t_struct*)type, name);
    } else if (type->is_container()) {
        generate_deserialize_container(out, type, name);
    } else if (type->is_base_type()) {
        indent(out);
        t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
        switch (tbase) {
        case t_base_type::TYPE_VOID:
            throw "compiler error: cannot serialize void field in a struct: " + name;
            break;
        case t_base_type::TYPE_STRING:
            out << name << " = field.valueString;";
            break;
        case t_base_type::TYPE_BOOL:
            out << name << " = field.valueBool;";
            break;
        case t_base_type::TYPE_BYTE:
            // FIXME: no implemented in reader
            out << name << " = field.valueByte;";
            throw "CANNOT GENERATE DESERIALIZE CODE FOR byte TYPE: " + type->get_name();
            break;
        case t_base_type::TYPE_I16:
            out << name << " = field.valueInt16;";
            break;
        case t_base_type::TYPE_I32:
            out << name << " = field.valueInt32;";
            break;
        case t_base_type::TYPE_I64:
            out << name << " = field.valueInt64;";
            break;
        case t_base_type::TYPE_DOUBLE:
            // FIXME: no implemented in reader
            out << name << " = field.valueDouble;";
            throw "CANNOT GENERATE DESERIALIZE CODE FOR double TYPE: " + type->get_name();
            break;
        default:
            throw "compiler error: no C++ reader for base type " + t_base_type::t_base_name(tbase) + name;
        }
        out <<
        endl;
    } else if (type->is_enum()) {
        out << name << " = (" << type_name(type) << ")field.valueInt32;" << endl;
    } else {
        printf("DO NOT KNOW HOW TO DESERIALIZE TYPE '%s'\n",
            type_name(type).c_str());
    }
}

void t_cppemb_generator::generate_deserialize_field(ofstream& out,
    t_field* tfield,
    string prefix,
    string suffix)
{
    t_type* type = get_true_type(tfield->get_type());
    
    if (type->is_void()) {
        throw "CANNOT GENERATE DESERIALIZE CODE FOR void TYPE: " +
        prefix + tfield->get_name();
    }
    
    string name = prefix + tfield->get_name() + suffix;
    
    generate_deserialize_type( out, type, name );
}

/**
* Generates an unserializer for a variable. This makes two key assumptions,
* first that there is a const char* variable named data that points to the
* buffer for deserialization, and that there is a variable protocol which
* is a reference to a TProtocol serialization object.
*/
void t_cppemb_generator::generate_deserialize_struct(ofstream& out,
    t_struct* tstruct,
    string prefix)
{
   indent(out) << prefix << ".read(reader);" << endl;
}

void t_cppemb_generator::generate_deserialize_container(ofstream& out,
    t_type* ttype,
    string prefix)
{
    scope_up(out);
    
    string size = tmp("_size");
    string ktype = tmp("_ktype");
    string vtype = tmp("_vtype");
    string etype = tmp("_etype");
    
    t_container* tcontainer = (t_container*)ttype;
    bool use_push = tcontainer->has_cpp_name();
    
    indent(out) <<
    prefix << ".clear();" << endl <<
    indent() << "uint32_t " << size << ";" << endl;
    
    // Declare variables, read header
    if (ttype->is_map()) {
        out <<
        indent() << "::apache::thrift::protocol::TType " << ktype << ";" << endl <<
        indent() << "::apache::thrift::protocol::TType " << vtype << ";" << endl <<
        indent() << "xfer += iprot->readMapBegin(" <<
        ktype << ", " << vtype << ", " << size << ");" << endl;
    } else if (ttype->is_set()) {
        out <<
        indent() << "::apache::thrift::protocol::TType " << etype << ";" << endl <<
        indent() << "xfer += iprot->readSetBegin(" <<
        etype << ", " << size << ");" << endl;
    } else if (ttype->is_list()) {
        out <<
        indent() << "::apache::thrift::protocol::TType " << etype << ";" << endl <<
        indent() << "xfer += iprot->readListBegin(" <<
        etype << ", " << size << ");" << endl;
        if (!use_push) {
            indent(out) << prefix << ".resize(" << size << ");" << endl;
        }
    }
    
    
    // For loop iterates over elements
    string i = tmp("_i");
    out <<
    indent() << "uint32_t " << i << ";" << endl <<
    indent() << "for (" << i << " = 0; " << i << " < " << size << "; ++" << i << ")" << endl;
    
    scope_up(out);
    
    if (ttype->is_map()) {
        generate_deserialize_map_element(out, (t_map*)ttype, prefix);
    } else if (ttype->is_set()) {
        generate_deserialize_set_element(out, (t_set*)ttype, prefix);
    } else if (ttype->is_list()) {
        generate_deserialize_list_element(out, (t_list*)ttype, prefix, use_push, i);
    }
    
    scope_down(out);
    
    // Read container end
    if (ttype->is_map()) {
        indent(out) << "xfer += iprot->readMapEnd();" << endl;
    } else if (ttype->is_set()) {
        indent(out) << "xfer += iprot->readSetEnd();" << endl;
    } else if (ttype->is_list()) {
        indent(out) << "xfer += iprot->readListEnd();" << endl;
    }
    
    scope_down(out);
}


/**
* Generates code to deserialize a map
*/
void t_cppemb_generator::generate_deserialize_map_element(ofstream& out,
    t_map* tmap,
    string prefix)
{
    string key = tmp("_key");
    string val = tmp("_val");
    t_field fkey(tmap->get_key_type(), key);
    t_field fval(tmap->get_val_type(), val);
    
    out <<
    indent() << declare_field(&fkey) << endl;
    
    generate_deserialize_field(out, &fkey);
    indent(out) <<
    declare_field(&fval, false, false) << " = " <<
    prefix << "[" << key << "];" << endl;
    
    generate_deserialize_field(out, &fval);
}

void t_cppemb_generator::generate_deserialize_set_element(ofstream& out,
    t_set* tset,
    string prefix)
{
    string elem = tmp("_elem");
    t_field felem(tset->get_elem_type(), elem);
    
    indent(out) <<
    declare_field(&felem) << endl;
    
    generate_deserialize_field(out, &felem);
    
    indent(out) <<
    prefix << ".insert(" << elem << ");" << endl;
}

void t_cppemb_generator::generate_deserialize_list_element(ofstream& out,
    t_list* tlist,
    string prefix,
    bool use_push,
    string index)
{
    if (use_push) {
        string elem = tmp("_elem");
        t_field felem(tlist->get_elem_type(), elem);
        indent(out) << declare_field(&felem) << endl;
        generate_deserialize_field(out, &felem);
        indent(out) << prefix << ".push_back(" << elem << ");" << endl;
    } else {
        t_field felem(tlist->get_elem_type(), prefix + "[" + index + "]");
        generate_deserialize_field(out, &felem);
    }
}


/**
* Serializes a field of any type.
*
* @param tfield The field to serialize
* @param prefix Name to prepend to field name
*/
void t_cppemb_generator::generate_serialize_type(ofstream& out,
    t_type* type,
    string name,
    int32_t field_id
    )
{
    
    // Do nothing for void types
    if (type->is_void()) {
        throw "CANNOT GENERATE SERIALIZE CODE FOR void TYPE: " + name;
    }
    
    
    
    if (type->is_struct() || type->is_xception()) {
        generate_serialize_struct(out,
            (t_struct*)type,
            name);
    } else if (type->is_container()) {
        generate_serialize_container(out, type, name);
    } else if (type->is_base_type() || type->is_enum()) {
        
        indent(out) << "writer.";
        
        if (type->is_base_type()) {
            t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
            switch (tbase) {
            case t_base_type::TYPE_VOID:
                throw
                "compiler error: cannot serialize void field in a struct: " + name;
                break;
            case t_base_type::TYPE_STRING:
                if (((t_base_type*)type)->is_binary()) {
                    throw "compiler error: no C++ writer for base type " + t_base_type::t_base_name(tbase) + name;
                }
                else {
                    out << "addString(" << field_id << ",reinterpret_cast<const char*>(" << name << ".buffer), " << name << ".length);" << endl;
                }
                break;
            case t_base_type::TYPE_BOOL:
                out << "addBool(" << field_id << "," << name << ");" << endl;
                break;
            case t_base_type::TYPE_BYTE:
                throw "compiler error: no C++ writer for base type " + t_base_type::t_base_name(tbase) + name;
                break;
            case t_base_type::TYPE_I16:
                out << "addInt16(" << field_id << "," << name << ");" << endl;
                break;
            case t_base_type::TYPE_I32:
                out << "addInt32(" << field_id << "," << name << ");" << endl;
                break;
            case t_base_type::TYPE_I64:
                out << "addInt64(" << field_id << "," << name << ");" << endl;
                break;
            case t_base_type::TYPE_DOUBLE:
                throw "compiler error: no C++ writer for base type " + t_base_type::t_base_name(tbase) + name;
                break;
            default:
                throw "compiler error: no C++ writer for base type " + t_base_type::t_base_name(tbase) + name;
            }
        } else if (type->is_enum()) {
                out << "addInt32(" << field_id << ", (int32_t)" << name << ");" << endl;
        }
    } else {
        printf("DO NOT KNOW HOW TO SERIALIZE FIELD '%s' TYPE '%s'\n",
            name.c_str(),
            type_name(type).c_str());
    }
}


void t_cppemb_generator::generate_serialize_field(ofstream& out,
    t_field* tfield,
    string prefix,
    string suffix)
{
    t_type* type = get_true_type(tfield->get_type());
    
    string name = prefix + tfield->get_name() + suffix;
    if( tfield->get_req() != t_field::T_REQUIRED )
    {
        indent(out) << "if( isSet_" << name << " )" << endl;
        indent_up();
    }
    generate_serialize_type(out,type,name,tfield->get_key());
    if( tfield->get_req() != t_field::T_REQUIRED )
    {
        indent_down();
    }
}

/**
* Serializes all the members of a struct.
*
* @param tstruct The struct to serialize
* @param prefix  String prefix to attach to all fields
*/
void t_cppemb_generator::generate_serialize_struct(ofstream& out,
    t_struct* tstruct,
    string prefix)
{
    indent(out) << prefix << ".write(writer);" << endl;
}

void t_cppemb_generator::generate_serialize_container(ofstream& out,
    t_type* ttype,
    string prefix)
{
    throw std::string("compiler error: cannot serialize containers");
    #if 0
    scope_up(out);
    
    if (ttype->is_map()) {
        indent(out) <<
        "xfer += oprot->writeMapBegin(" <<
        type_to_enum(((t_map*)ttype)->get_key_type()) << ", " <<
        type_to_enum(((t_map*)ttype)->get_val_type()) << ", " <<
        "static_cast<uint32_t>(" << prefix << ".size()));" << endl;
    } else if (ttype->is_set()) {
        indent(out) <<
        "xfer += oprot->writeSetBegin(" <<
        type_to_enum(((t_set*)ttype)->get_elem_type()) << ", " <<
        "static_cast<uint32_t>(" << prefix << ".size()));" << endl;
    } else if (ttype->is_list()) {
        indent(out) <<
        "xfer += oprot->writeListBegin(" <<
        type_to_enum(((t_list*)ttype)->get_elem_type()) << ", " <<
        "static_cast<uint32_t>(" << prefix << ".size()));" << endl;
    }
    
    string iter = tmp("_iter");
    out <<
    indent() << type_name(ttype) << "::const_iterator " << iter << ";" << endl <<
    indent() << "for (" << iter << " = " << prefix  << ".begin(); " << iter << " != " << prefix << ".end(); ++" << iter << ")" << endl;
    scope_up(out);
    if (ttype->is_map()) {
        generate_serialize_map_element(out, (t_map*)ttype, iter);
    } else if (ttype->is_set()) {
        generate_serialize_set_element(out, (t_set*)ttype, iter);
    } else if (ttype->is_list()) {
        generate_serialize_list_element(out, (t_list*)ttype, iter);
    }
    scope_down(out);
    
    if (ttype->is_map()) {
        indent(out) <<
        "xfer += oprot->writeMapEnd();" << endl;
    } else if (ttype->is_set()) {
        indent(out) <<
        "xfer += oprot->writeSetEnd();" << endl;
    } else if (ttype->is_list()) {
        indent(out) <<
        "xfer += oprot->writeListEnd();" << endl;
    }
    
    scope_down(out);
    #endif
}

/**
* Serializes the members of a map.
*
*/
void t_cppemb_generator::generate_serialize_map_element(ofstream& out,
    t_map* tmap,
    string iter)
{
    t_field kfield(tmap->get_key_type(), iter + "->first");
    generate_serialize_field(out, &kfield, "");
    
    t_field vfield(tmap->get_val_type(), iter + "->second");
    generate_serialize_field(out, &vfield, "");
}

/**
* Serializes the members of a set.
*/
void t_cppemb_generator::generate_serialize_set_element(ofstream& out,
    t_set* tset,
    string iter)
{
    t_field efield(tset->get_elem_type(), "(*" + iter + ")");
    generate_serialize_field(out, &efield, "");
}

/**
* Serializes the members of a list.
*/
void t_cppemb_generator::generate_serialize_list_element(ofstream& out,
    t_list* tlist,
    string iter)
{
    t_field efield(tlist->get_elem_type(), "(*" + iter + ")");
    generate_serialize_field(out, &efield, "");
}

/**
* Makes a :: prefix for a namespace
*
* @param ns The namespace, w/ periods in it
* @return Namespaces
*/
string t_cppemb_generator::namespace_prefix(string ns) {
    // Always start with "::", to avoid possible name collisions with
    // other names in one of the current namespaces.
    //
    // We also need a leading space, in case the name is used inside of a
    // template parameter.  "MyTemplate<::foo::Bar>" is not valid C++,
    // since "<:" is an alternative token for "[".
    string result = " ::";
    
    if (ns.size() == 0) {
        return result;
    }
    string::size_type loc;
    while ((loc = ns.find(".")) != string::npos) {
        result += ns.substr(0, loc);
        result += "::";
        ns = ns.substr(loc+1);
    }
    if (ns.size() > 0) {
        result += ns + "::";
    }
    return result;
}

/**
* Opens namespace.
*
* @param ns The namespace, w/ periods in it
* @return Namespaces
*/
string t_cppemb_generator::namespace_open(string ns) {
    if (ns.size() == 0) {
        return "";
    }
    string result = "";
    string separator = "";
    string::size_type loc;
    while ((loc = ns.find(".")) != string::npos) {
        result += separator;
        result += "namespace ";
        result += ns.substr(0, loc);
        result += " {";
        separator = " ";
        ns = ns.substr(loc+1);
    }
    if (ns.size() > 0) {
        result += separator + "namespace " + ns + " {";
    }
    if( use_embedded_namespace_ )
        result += separator + "namespace embedded {";
    return result;
}

/**
* Closes namespace.
*
* @param ns The namespace, w/ periods in it
* @return Namespaces
*/
string t_cppemb_generator::namespace_close(string ns) {
    if (ns.size() == 0) {
        return "";
    }
    string result = "}";
    if( use_embedded_namespace_ )
        result += "}";
    string::size_type loc;
    while ((loc = ns.find(".")) != string::npos) {
        result += "}";
        ns = ns.substr(loc+1);
    }
    result += " // namespace";
    return result;
}

/**
* Returns a C++ type name
*
* @param ttype The type
* @return String of the type name, i.e. std::set<type>
*/
string t_cppemb_generator::type_name(t_type* ttype, bool in_typedef, bool arg) {
    if (ttype->is_base_type()) {
        string bname = base_type_name(((t_base_type*)ttype)->get_base());
        std::map<string, string>::iterator it = ttype->annotations_.find("cpp.type");
        if (it != ttype->annotations_.end()) {
            bname = it->second;
        }
        
        if (!arg) {
            return bname;
        }
        
        if (((t_base_type*)ttype)->get_base() == t_base_type::TYPE_STRING) {
            return "const " + bname + "&";
        } else {
            return "const " + bname;
        }
    }
    
    // Check for a custom overloaded C++ name
    if (ttype->is_container()) {
        string cname;
        
        t_container* tcontainer = (t_container*) ttype;
        if (tcontainer->has_cpp_name()) {
            cname = tcontainer->get_cpp_name();
        } else if (ttype->is_map()) {
            t_map* tmap = (t_map*) ttype;
            cname = "std::map<" +
            type_name(tmap->get_key_type(), in_typedef) + ", " +
            type_name(tmap->get_val_type(), in_typedef) + "> ";
        } else if (ttype->is_set()) {
            t_set* tset = (t_set*) ttype;
            cname = "std::set<" + type_name(tset->get_elem_type(), in_typedef) + "> ";
        } else if (ttype->is_list()) {
            t_list* tlist = (t_list*) ttype;
            cname = "std::vector<" + type_name(tlist->get_elem_type(), in_typedef) + "> ";
        }
        
        if (arg) {
            return "const " + cname + "&";
        } else {
            return cname;
        }
    }
    
    string class_prefix;
    if (in_typedef && (ttype->is_struct() || ttype->is_xception())) {
        class_prefix = "class ";
    }
    
    // Check if it needs to be namespaced
    string pname;
    t_program* program = ttype->get_program();
    if (program != NULL && program != program_) {
        pname =
        class_prefix +
        namespace_prefix(program->get_namespace("cpp")) +
        ttype->get_name();
    } else {
        pname = class_prefix + ttype->get_name();
    }
    
    if (arg) {
        if (is_complex_type(ttype)) {
            return "const " + pname + "&";
        } else {
            return "const " + pname;
        }
    } else {
        return pname;
    }
}

/**
* Returns the C++ type that corresponds to the thrift type.
*
* @param tbase The base type
* @return Explicit C++ type, i.e. "int32_t"
*/
string t_cppemb_generator::base_type_name(t_base_type::t_base tbase) {
    switch (tbase) {
    case t_base_type::TYPE_VOID:
        return "void";
    case t_base_type::TYPE_STRING:
        return "ConstBufferReference";
    case t_base_type::TYPE_BOOL:
        return "bool";
    case t_base_type::TYPE_BYTE:
        return "int8_t";
    case t_base_type::TYPE_I16:
        return "int16_t";
    case t_base_type::TYPE_I32:
        return "int32_t";
    case t_base_type::TYPE_I64:
        return "int64_t";
    case t_base_type::TYPE_DOUBLE:
        return "double";
    default:
        throw "compiler error: no C++ base type name for base type " + t_base_type::t_base_name(tbase);
    }
}

/**
* Declares a field, which may include initialization as necessary.
*
* @param ttype The type
* @return Field declaration, i.e. int x = 0;
*/
string t_cppemb_generator::declare_field(t_field* tfield, bool init, bool constant) {
    // TODO(mcslee): do we ever need to initialize the field?
    string result = "";
    if (constant) {
        result += "const ";
    }
    result += type_name(tfield->get_type());
    result += " " + tfield->get_name();
    if (init) {
        t_type* type = get_true_type(tfield->get_type());
        
        if (type->is_base_type()) {
            t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
            switch (tbase) {
            case t_base_type::TYPE_VOID:
            case t_base_type::TYPE_STRING:
                break;
            case t_base_type::TYPE_BOOL:
                result += " = false";
                break;
            case t_base_type::TYPE_BYTE:
            case t_base_type::TYPE_I16:
            case t_base_type::TYPE_I32:
            case t_base_type::TYPE_I64:
                result += " = 0";
                break;
            case t_base_type::TYPE_DOUBLE:
                result += " = (double)0";
                break;
            default:
                throw "compiler error: no C++ initializer for base type " + t_base_type::t_base_name(tbase);
            }
        } else if (type->is_enum()) {
            result += " = (" + type_name(type) + ")0";
        }
    }
    result += ";";
    return result;
}

/**
* Renders a function signature of the form 'type name(args)'
*
* @param tfunction Function definition
* @return String of rendered function definition
*/
string t_cppemb_generator::function_signature(t_function* tfunction,
    string prefix,
    bool name_params)
{
    t_type* ttype = tfunction->get_returntype();
    t_struct* arglist = tfunction->get_arglist();
    bool has_xceptions = !tfunction->get_xceptions()->get_members().empty();
    
    if (is_complex_type(ttype)) {
        return
        "void " + prefix + tfunction->get_name() +
        "(" + type_name(ttype) + (name_params ? "& _return" : "& /* _return */") +
        argument_list(arglist, name_params, true) + ")";
    } else {
        return
        type_name(ttype) + " " + prefix + tfunction->get_name() +
        "(" + argument_list(arglist, name_params) + ")";
    }
}

/**
* Renders a field list
*
* @param tstruct The struct definition
* @return Comma sepearated list of all field names in that struct
*/
string t_cppemb_generator::argument_list(t_struct* tstruct, bool name_params, bool start_comma, bool insert_isset, bool type_params )
{
    string result = "";
    
    const vector<t_field*>& fields = tstruct->get_members();
    vector<t_field*>::const_iterator f_iter;
    bool first = !start_comma;
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
        if (first) {
            first = false;
        } else {
            result += ", ";
        }
        if( type_params )
            result += type_name((*f_iter)->get_type(), false, true) + " ";
        result += (name_params ? (*f_iter)->get_name() : "/* " + (*f_iter)->get_name() + " */");
        if( insert_isset && (*f_iter)->get_req() != t_field::T_REQUIRED )
            result += (type_params ? ", bool " : ", ") + (name_params ? "isSet_" + (*f_iter)->get_name() : "/* isSet_" + (*f_iter)->get_name() + " */");
    }
    return result;
}

/**
* Converts the parse type to a C++ enum string for the given type.
*
* @param type Thrift Type
* @return String of C++ code to definition of that type constant
*/
string t_cppemb_generator::type_to_enum(t_type* type) {
    type = get_true_type(type);
    
    if (type->is_base_type()) {
        t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
        switch (tbase) {
        case t_base_type::TYPE_VOID:
            throw "NO T_VOID CONSTRUCT";
        case t_base_type::TYPE_STRING:
            return "::Thrift::FieldType::STRING";
        case t_base_type::TYPE_BOOL:
            return "::Thrift::FieldType::BOOL";
        case t_base_type::TYPE_BYTE:
            return "::Thrift::FieldType::BYTE";
        case t_base_type::TYPE_I16:
            return "::Thrift::FieldType::I16";
        case t_base_type::TYPE_I32:
            return "::Thrift::FieldType::I32";
        case t_base_type::TYPE_I64:
            return "::Thrift::FieldType::I64";
        case t_base_type::TYPE_DOUBLE:
            return "::Thrift::FieldType::DOUBLE";
        }
    } else if (type->is_enum()) {
        return "::Thrift::FieldType::I32";
    } else if (type->is_struct()) {
        return "::Thrift::FieldType::STRUCT";
    } else if (type->is_xception()) {
        return "::Thrift::FieldType::STRUCT";
    } else if (type->is_map()) {
        return "::Thrift::FieldType::MAP";
    } else if (type->is_set()) {
        return "::Thrift::FieldType::SET";
    } else if (type->is_list()) {
        return "::Thrift::FieldType::LIST";
    }
    
    throw "INVALID TYPE IN type_to_enum: " + type->get_name();
}

string t_cppemb_generator::get_include_prefix(const t_program& program) const {
    string include_prefix = program.get_include_prefix();
    if (!use_include_prefix_ ||
        (include_prefix.size() > 0 && include_prefix[0] == '/')) {
    // if flag is turned off or this is absolute path, return empty prefix
    return "";
        }
        
        string::size_type last_slash = string::npos;
        if ((last_slash = include_prefix.rfind("/")) != string::npos) {
            return include_prefix.substr(0, last_slash) +
            (get_program()->is_out_path_absolute() ? "/" : "/" + out_dir_base_ + "/");
            
        }
        
        return "";
}


THRIFT_REGISTER_GENERATOR(cppemb, "C++ embedded",
    "    include_prefix:       Use full include paths in generated files.\n"
    "    embedded_namespace:   Add an extra namespace embedded to use.\n"
    )

