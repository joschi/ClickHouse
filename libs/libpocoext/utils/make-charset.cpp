/**
 * @file
 * @author Sergey N. Yatskevich<snc@altlinux.ru>
 * @brief
 */
/*
 * $Id$
 */
#include <cstdio>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <string>
#include <list>

class Encoding
{
	std::list<std::string> m_names;

	int m_charset[256];
	unsigned char m_reverse_charset[256][256];

	void buildNamesTable (std::ostream& _os, const std::string& _class_name) const;
	void buildMap (std::ostream& _os, const std::string& _class_name) const;
	void buildReverseMap (std::ostream& _os, const std::string& _class_name) const;

public:
	Encoding ()
	{
		for (int i = 0; i < 256; ++i)
			m_charset[i] = -1;

		memset (m_reverse_charset, 0, sizeof (m_reverse_charset));
	};

	void read (std::istream& _is);

	void buildHead (std::ostream& _os, const std::string& _class_name) const;
	void buildBody (std::ostream& _os, const std::string& _class_name) const;
};

void
Encoding::read (std::istream& _is)
{
	// Read stream and build charset and name list
	while (_is.good ())
	{
		std::string line;
		std::getline (_is, line);

		char     s[1024]; // string
		int      c;       // 8-bit char
		long int uc;      // unicode (32-bit) char

		if (std::sscanf (line.c_str (), "%i %li", &c, &uc) == 2)
		{
			if ((c < 0) || (c > 255) || (uc < 0) || ((uc > 0xFEFE) && (uc != 0xFFFE)))
				throw std::runtime_error ("Invalid charset file");

			m_charset[c] = uc;
		}

		else if (std::sscanf (line.c_str (), "canonical is a %s", s) == 1)
			m_names.push_front (s);

		else if (std::sscanf (line.c_str (), "alias is a %s", s) == 1)
			m_names.push_back (s);
	}

	// Build reverse map
	for (int i = 0; i < 256; ++i)
	{
		if (m_charset[i] >= 0)
			m_reverse_charset[(m_charset[i] >> 8) & 0xff][m_charset[i] & 0xff] = i;
	}
}

void
Encoding::buildNamesTable (std::ostream& _os, const std::string& _class_name) const
{
	_os << "const char* Poco::" << _class_name << "::_names[] =\n"
			"{\n";
	for (std::list<std::string>::const_iterator n = m_names.begin (); n != m_names.end (); ++n)
		_os << "\t\"" << *n << "\",\n";
	_os << "\tNULL\n"
			"};\n\n\n";
}

void
Encoding::buildMap (std::ostream& _os, const std::string& _class_name) const
{
	_os << "const Poco::TextEncoding::CharacterMap Poco::" << _class_name << "::_map =\n"
			"{\n";
	for (int i = 0; i < 256; ++i)
	{
		if ((i & 0x0f) == 0x00)
			_os << '\t';

		if (m_charset[i] >= 0)
		{
			_os << "0x"
				<< std::hex << std::setw (4) << std::setfill ('0')
				<< m_charset[i] << ", ";
		}
		else
		{
			_os
				<< std::dec << std::setw (6) << std::setfill (' ')
				<< m_charset[i] << ", ";
		}

		if ((i & 0x0f) == 0x0f)
			_os << std::endl;
	}
	_os << "};\n\n\n";
}

void
Encoding::buildReverseMap (std::ostream& _os, const std::string& _class_name) const
{
	for (int i = 0; i < 256; ++i)
	{
		int j = 0;
		for (; j < 256; ++j)
		{
			if (m_reverse_charset[i][j] != 0)
				break;
		}

		// skip "empty" lines
		if (j == 256)
			continue;

		_os << "const unsigned char Poco::" << _class_name << "::_0x"
			<< std::hex << std::setw (4) << std::setfill ('0')
			<< (i << 8) << "_map[256] =\n{\n";

		for (int j = 0; j < 256; ++j)
		{
			if ((j & 0x0f) == 0x00)
				_os << '\t';

			if (m_reverse_charset[i][j] > 0)
			{
				_os << "0x"
					<< std::hex << std::setw (2) << std::setfill ('0')
					<< (unsigned int)m_reverse_charset[i][j] << ", ";
			}
			else
			{
				_os
					<< std::dec << std::setw (4) << std::setfill (' ')
					<< (unsigned int)m_reverse_charset[i][j] << ", ";
			}

			if ((j & 0x0f) == 0x0f)
				_os << std::endl;
		}
		_os << "};\n\n\n";
	}
}

void
Encoding::buildHead (std::ostream& _os, const std::string& _class_name) const
{
	_os <<
		"// \n"
		"// " << _class_name << ".h\n"
		"// \n"
		"// $Id$\n"
		"// \n"
		"// Library: Foundation\n"
		"// Package: Text\n"
		"// Module:  " << _class_name << "\n"
		"// \n"
		"// Definition of the " << _class_name << " class.\n"
		"// \n"
		"// Copyright (c) 2004-2007, Applied Informatics Software Engineering GmbH.\n"
		"// and Contributors.\n"
		"// \n"
		"// Permission is hereby granted, free of charge, to any person or organization\n"
		"// obtaining a copy of the software and accompanying documentation covered by\n"
		"// this license (the \"Software\") to use, reproduce, display, distribute,\n"
		"// execute, and transmit the Software, and to prepare derivative works of the\n"
		"// Software, and to permit third-parties to whom the Software is furnished to\n"
		"// do so, all subject to the following:\n"
		"// \n"
		"// The copyright notices in the Software and this entire statement, including\n"
		"// the above license grant, this restriction and the following disclaimer,\n"
		"// must be included in all copies of the Software, in whole or in part, and\n"
		"// all derivative works of the Software, unless such copies or derivative\n"
		"// works are solely in the form of machine-executable object code generated by\n"
		"// a source language processor.\n"
		"// \n"
		"// THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
		"// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
		"// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT\n"
		"// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE\n"
		"// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,\n"
		"// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER\n"
		"// DEALINGS IN THE SOFTWARE.\n"
		"//\n"
		"// This file is generated automatically. Do not edit it.\n"
		"//\n\n\n"
		"#pragma once\n\n\n";

	_os << "#include <string>\n";
	_os << "#include <Poco/Foundation.h>\n";
	_os << "#include <Poco/TextEncoding.h>\n\n\n";

	_os << "namespace Poco\n{\n";

	_os << "\tclass Foundation_API " << _class_name << " : public Poco::TextEncoding\n";
	_os << "\t{\n";
	_os << "\t\tstatic const char* _names[];\n";
	_os << "\t\tstatic const CharacterMap _map;\n";

	for (int i = 0; i < 256; ++i)
	{
		int j = 0;
		for (; j < 256; ++j)
		{
			if (m_reverse_charset[i][j] != 0)
				break;
		}

		// skip empty lines
		if (j == 256)
			continue;

		_os << "\t\tstatic const unsigned char _0x"
			<< std::hex << std::setw (4) << std::setfill ('0') << (i << 8)
			<< "_map[256];\n";
	}

	_os << "\n\tpublic:\n"
			"\t\t" << _class_name << "();\n"
			"\t\t~" << _class_name << "();\n\n";
	_os << "\t\tconst char* canonicalName() const;\n";
	_os << "\t\tbool isA(const std::string& encodingName) const;\n";
	_os << "\t\tconst CharacterMap& characterMap() const;\n";
	_os << "\t\tint convert(const unsigned char* bytes) const;\n";
	_os << "\t\tint queryConvert(const unsigned char* bytes, int length) const;\n";
	_os << "\t\tint convert(int ch, unsigned char* bytes, int length) const;\n";
	_os << "\t};\n";
	_os << "}\n\n";
}

void
Encoding::buildBody (std::ostream& _os, const std::string& _class_name) const
{
	_os <<
		"// \n"
		"// " << _class_name << ".cpp\n"
		"// \n"
		"// $Id$\n"
		"// \n"
		"// Library: Foundation\n"
		"// Package: Text\n"
		"// Module:  " << _class_name << "\n"
		"// \n"
		"// Copyright (c) 2004-2007, Applied Informatics Software Engineering GmbH.\n"
		"// and Contributors.\n"
		"// \n"
		"// Permission is hereby granted, free of charge, to any person or organization\n"
		"// obtaining a copy of the software and accompanying documentation covered by\n"
		"// this license (the \"Software\") to use, reproduce, display, distribute,\n"
		"// execute, and transmit the Software, and to prepare derivative works of the\n"
		"// Software, and to permit third-parties to whom the Software is furnished to\n"
		"// do so, all subject to the following:\n"
		"// \n"
		"// The copyright notices in the Software and this entire statement, including\n"
		"// the above license grant, this restriction and the following disclaimer,\n"
		"// must be included in all copies of the Software, in whole or in part, and\n"
		"// all derivative works of the Software, unless such copies or derivative\n"
		"// works are solely in the form of machine-executable object code generated by\n"
		"// a source language processor.\n"
		"// \n"
		"// THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
		"// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
		"// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT\n"
		"// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE\n"
		"// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,\n"
		"// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER\n"
		"// DEALINGS IN THE SOFTWARE.\n"
		"//\n"
		"// This file is generated automatically. Do not edit it.\n"
		"//\n\n\n";

	_os << "#include <Poco/String.h>\n";
	_os << "#include <Poco/" << _class_name << ".h>\n\n\n";

	buildNamesTable (_os, _class_name);
	buildMap (_os, _class_name);
	buildReverseMap (_os, _class_name);

	_os << "Poco::" << _class_name << "::" << _class_name << "()\n{}\n\n\n";
	_os << "Poco::" << _class_name << "::~" << _class_name << "()\n{}\n\n\n";

	_os << "const char* Poco::" << _class_name << "::canonicalName() const\n"
			"{\n"
				"\treturn _names[0];\n"
			"}\n\n\n";

	_os << "bool Poco::" << _class_name << "::isA(const std::string& encodingName) const\n"
			"{\n"
				"\tfor (const char** name = _names; *name; ++name)\n"
				"\t{\n"
					"\t\tif (Poco::icompare (encodingName, *name) == 0)\n"
						"\t\t\treturn true;\n"
				"\t}\n\n"
				"\treturn false;\n"
			"}\n\n\n";

	_os << "const Poco::TextEncoding::CharacterMap& Poco::" << _class_name << "::characterMap() const\n"
			"{\n"
				"\treturn _map;\n"
			"}\n\n\n";

	_os << "int Poco::" << _class_name << "::convert(const unsigned char* bytes) const\n"
			"{\n"
				"\treturn _map[*bytes];\n"
			"}\n\n\n";
			
	_os << "int Poco::" << _class_name << "::queryConvert(const unsigned char* bytes, int length) const\n"
			"{\n"
				"\treturn _map[*bytes];\n"
			"}\n\n\n";

	_os << "int Poco::" << _class_name << "::convert(int ch, unsigned char* bytes, int length) const\n"
			"{\n";

	_os << "\tif (ch == 0x0000)\n"
			"\t{\n"
				"\t\tif (bytes && (length >= 1))\n"
					"\t\t\t*bytes = 0x00;\n"
				"\t\treturn 1;\n"
			"\t}\n\n"
			"\telse";
	for (int i = 0; i < 256; ++i)
	{
		int j = 0;
		for (; j < 256; ++j)
		{
			if (m_reverse_charset[i][j] != 0)
				break;
		}

		// Empty line
		if (j == 256)
			continue;

		_os << " if ((ch & 0xff00) == 0x" << std::hex << std::setw (4) << std::setfill ('0') << (i << 8) << ")\n"
				"\t{\n"
					"\t\tif ((unsigned char)_0x" << std::hex << std::setw (4) << std::setfill ('0') << (i << 8) <<  "_map[ch & 0x00ff] > 0)\n"
					"\t\t{\n"
						"\t\t\tif (bytes && (length >= 1))\n"
							"\t\t\t\t*bytes = (unsigned char)_0x" << std::hex << std::setw (4) << std::setfill ('0') << (i << 8) <<  "_map[ch & 0x00ff];\n"
						"\t\t\treturn 1;\n"
					"\t\t}\n"
					"\t\treturn 0;\n"
				"\t}\n\n"
				"\telse";
	}
	_os << "\n\t\treturn 0;\n"
		"}\n";
}

int
main (int _argc, char* _argv[])
{
	Encoding e;

	e.read (std::cin);

	std::string class_name;
	std::string cpp_name;
	std::string h_name;
	if (_argc > 1)
	{
	    class_name = std::string(_argv[1]);
	    if (_argc > 2)
	    {
		cpp_name = std::string(_argv[2]);
		if (_argc > 3)
		    h_name = std::string(_argv[3]);
		else
		    h_name = cpp_name.substr(0, cpp_name.size() - 3) + "h";
	    }
	    else
	    {
	        cpp_name = class_name + ".cpp";
		h_name = class_name + ".h";
	    }
	}
	else
	{
	    class_name = "UNDEFINED";
	    cpp_name = "UNDEFINED.cpp";
	    h_name = "UNDEFINED.h";
	}

	std::ofstream head (h_name.c_str());
	std::ofstream body (cpp_name.c_str());

	e.buildHead (head, class_name);
	e.buildBody (body, class_name);

	return 0;
}
