/* 
 * File:   TypeSystemUtilities.hpp
 * Author: Michael Goulet
 *
 * Created on February 27, 2013, 7:33 PM
 */

#ifndef TYPESYSTEMUTILITIES_HPP
#define	TYPESYSTEMUTILITIES_HPP

class CStrHash {
public:
    int operator()(const char* str) const {
        int hash = 0;
        for (const char* i = str; *i != '\0'; i++)
            hash = ((hash << 5) ^ hash) ^ *i;
        return hash;
    }
};

class CStrEql {
public:
    bool operator()(const char* const& a, const char* const& b) const {
        int i;
        for (i = 0; a[i] != '\0' && b[i] != '\0'; i++) { } //find the first one
        return (a[i] == '\0') && (b[i] == '\0');
    }
};

#endif	/* TYPESYSTEMUTILITIES_HPP */

