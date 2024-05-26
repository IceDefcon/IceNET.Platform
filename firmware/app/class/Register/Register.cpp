//
// Created by Unknown on 2024/05/25.
// Copyright (c) 2024 Unknown. All rights reserved.
//

#include "Register.h"
#include <vector>
#include <stdio.h>

// Konstruktor
myRegister::myRegister() : address(0xFF), value(0xFF), ptrName("Default"){
    this->print();
}

myRegister::myRegister(const char addr, const char val, const char *const ptrString) : address(addr), value(val), ptrName(ptrString){
    this->print();
}

// Destruktor
myRegister::~myRegister() {
    // Kod czyszczący
}

// Metoda do odczytu adresu rejestru
char myRegister::getAddress() {
    return this->address;
}

// Metoda do odczytu rozmiaru rejestru
char myRegister::getValue() {
    return this->value;
}

// Implementacja metody publicznej
void myRegister::print() {
    printf("myRegister: addr:0x%x, val:0x%x, name:%s", this->address, this->value, this->ptrName);
    // std::cout << "address publiczna" << std::endl;
}
