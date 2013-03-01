/* 
 * File:   TypeSystemUtilities.hpp
 * Author: Michael Goulet
 *
 * Created on February 27, 2013, 7:33 PM
 */

#ifndef TYPESYSTEMUTILITIES_HPP
#define	TYPESYSTEMUTILITIES_HPP

#include <stddef.h>

#include "SyntaxTreeUtil.h"
#include "ParserNodes.h"

class CStrHash;
class CStrEql;
class LambdaHash;
class LambdaEql;

template<typename T>
class Array {
private:
    T* array;
    size_t length;
public:
    Array() {
        length = 0;
        array = NULL;
    }
    
    Array(size_t length) : length(length) {
        if (length == 0)
            array = NULL;
        else
            array = new T[length];
    }
    
    Array(const Array& copy) : length(copy.length) {
        array = new T[length];
        for (size_t i = 0; i < size(); i++) {
            array[i] = copy.array[i]; //direct copy, don't need index-out-of-bounds guards.
        }
    }
    
    ~Array() {
        delete[] array;
        array = NULL;
        length = -1;
    }
    
    T& operator[](size_t index) {
        if (index >= length) {
            printf("Error: Attempt to access array (size = %d) at index %d!\n", length, index);
            exit(0);
            return *array;
        }
        return array[index];
    }
    
    const T& operator[](size_t index) const {
        if (index >= length) {
            printf("Error: Attempt to access array (size = %d) at index %d!\n", length, index);
            exit(0);
            return *array;
        }
        return array[index];
    }
    
    Array& operator=(const Array& copy) {
        if (&copy == this)
            return *this;
        if (length != copy.length) {
            delete[] array;
            array = new T[copy.length];
            length = copy.length;
        }
        for (size_t i = 0; i < size(); i++) {
            array[i] = copy.array[i]; //direct copy, don't need index-out-of-bounds guards.
        }
        return *this;
    }
    
    size_t size() const {
        return length;
    }
};

typedef std::pair<CheshireType, Array<CheshireType> > LambdaType;

typedef std::unordered_map<const char*, TypeKey, CStrHash, CStrEql> TypeMap;
typedef std::unordered_map<LambdaType, TypeKey, LambdaHash, LambdaEql> LambdaMap;
typedef std::unordered_map<TypeKey, const char*> ValidObjectSet;
typedef std::unordered_map<TypeKey, LambdaType> ValidLambdaSet;

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

class LambdaHash {
public:
    int operator()(const LambdaType& lambda) const {
        int hash = lambda.first.typeKey;
        for (size_t i = 0; i < lambda.second.size(); i++) {
            hash = ((hash << 5) ^ hash) ^ int(lambda.second[i].typeKey);
        }
        return hash;
    }
};

class LambdaEql {
public:
    bool operator()(const LambdaType& a, const LambdaType& b) const {
        if (!typeEql(a.first, b.first))
            return false;
        if (a.second.size() != b.second.size())
            return false;
        for (size_t i = 0; i < a.second.size(); i++) {
            if (!typeEql(a.second[i], b.second[i]))
                return false;
        }
        return true;
    }
    
    bool typeEql(const CheshireType& a, const CheshireType& b) const {
        return (a.typeKey == b.typeKey) && (a.isUnsafe == b.isUnsafe) && (a.isInfer == b.isInfer);
    }
};

#endif	/* TYPESYSTEMUTILITIES_HPP */

