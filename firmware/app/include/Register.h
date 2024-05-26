//
// Created by Ventor92 on 2024/05/25.
// Copyright (c) 2024 Ventor92. All rights reserved.
//

#ifndef CLASS_REGISTER_H_
#define CLASS_REGISTER_H_

#include <cstring>

class myRegister {
 public:

    myRegister();
    myRegister(const char addr, const char val, const char *const ptrString);
    ~myRegister();

    char getAddress();
    char getValue();
    void print();

 private:
    const char address; // Adres rejestru
    char value;
    const char *const ptrName;
};

#endif  // CLASS_REGISTER_H_
