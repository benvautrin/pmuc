#ifndef STRINGUTILS_H
#define STRINGUTILS_H


// Fix for MinGW64
// TODO (MP) : check for a fix in MinGW64
template<typename T> std::string toString( const T& data ) {
	std::stringstream s;
	s << data;
	return s.str();
}


std::string escapeXMLAttribute(const std::string& value);

#endif // STRINGUTILS_H
