#include "chip8.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>

// Built-in Hexadecimal Font Display Specs
uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 
    0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 
    0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 
    0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40, 
    0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0, 
    0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0, 
    0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 
    0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80  
};

Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()), randByte(0, 255) {
    pc = START_ADDRESS;
    for (unsigned int i = 0; i < 80; ++i) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
    std::fill(std::begin(video), std::end(video), COLOR_BACKGROUND);
}

void Chip8::LoadROM(char const* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streampos size = file.tellg();
        char* buffer = new char[size];
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        for (long i = 0; i < size; ++i) {
            memory[START_ADDRESS + i] = buffer[i];
        }
        delete[] buffer;
        std::cout << "[SYSTEM] Game ROM loaded successfully." << std::endl;
    } else {
        std::cerr << "[ERROR] Could not open ROM file." << std::endl;
        exit(1);
    }
}

void Chip8::Cycle() {
    opcode = (memory[pc] << 8) | memory[pc + 1];
    pc += 2;

    uint8_t u = (opcode & 0xF000u) >> 12u;
    uint8_t x = (opcode & 0x0F00u) >> 8u;
    uint8_t y = (opcode & 0x00F0u) >> 4u;
    uint8_t n = opcode & 0x000Fu;
    uint8_t kk = opcode & 0x00FFu;
    uint16_t nnn = opcode & 0x0FFFu;

    switch (u) {
        case 0x0:
            if (kk == 0xE0) { 
                std::fill(std::begin(video), std::end(video), COLOR_BACKGROUND);
            } else if (kk == 0xEE) { 
                --sp; pc = stack[sp];
            }
            break;

        case 0x1: pc = nnn; break;
        case 0x2: stack[sp] = pc; ++sp; pc = nnn; break;
        case 0x3: if (registers[x] == kk) pc += 2; break;
        case 0x4: if (registers[x] != kk) pc += 2; break;
        case 0x5: if (registers[x] == registers[y]) pc += 2; break;
        case 0x6: registers[x] = kk; break;
        case 0x7: registers[x] += kk; break;

        case 0x8:
            switch (n) {
                case 0x0: registers[x] = registers[y]; break;
                case 0x1: registers[x] |= registers[y]; break;
                case 0x2: registers[x] &= registers[y]; break;
                case 0x3: registers[x] ^= registers[y]; break;
                case 0x4: {
                    uint16_t sum = registers[x] + registers[y];
                    registers[0xF] = (sum > 255u) ? 1 : 0;
                    registers[x] = sum & 0xFFu;
                } break;
                case 0x5:
                    registers[0xF] = (registers[x] > registers[y]) ? 1 : 0;
                    registers[x] -= registers[y];
                    break;
                case 0x6:
                    registers[0xF] = registers[x] & 0x1u;
                    registers[x] >>= 1;
                    break;
                case 0x7:
                    registers[0xF] = (registers[y] > registers[x]) ? 1 : 0;
                    registers[x] = registers[y] - registers[x];
                    break;
                case 0xE:
                    registers[0xF] = (registers[x] & 0x80u) >> 7u;
                    registers[x] <<= 1;
                    break;
            }
            break;

        case 0x9: if (registers[x] != registers[y]) pc += 2; break;
        case 0xA: index = nnn; break;
        case 0xB: pc = nnn + registers[0]; break;
        case 0xC: registers[x] = randByte(randGen) & kk; break;

        case 0xD: { 
            uint8_t xCoord = registers[x] % VIDEO_WIDTH;
            uint8_t yCoord = registers[y] % VIDEO_HEIGHT;
            registers[0xF] = 0;

            for (unsigned int row = 0; row < n; ++row) {
                uint8_t spriteByte = memory[index + row];
                for (unsigned int col = 0; col < 8; ++col) {
                    uint8_t spritePixel = spriteByte & (0x80u >> col);
                    if (spritePixel) {
                        unsigned int targetX = (xCoord + col) % VIDEO_WIDTH;
                        unsigned int targetY = (yCoord + row) % VIDEO_HEIGHT;
                        uint32_t* screenPixel = &video[targetY * VIDEO_WIDTH + targetX];

                        if (*screenPixel == COLOR_FOREGROUND) {
                            registers[0xF] = 1; 
                            *screenPixel = COLOR_BACKGROUND;
                        } else {
                            *screenPixel = COLOR_FOREGROUND;
                        }
                    }
                }
            }
        } break;

        case 0xE:
            if (kk == 0x9E) { if (keypad[registers[x]]) pc += 2; }
            else if (kk == 0xA1) { if (!keypad[registers[x]]) pc += 2; }
            break;

        case 0xF:
            switch (kk) {
                case 0x07: registers[x] = delayTimer; break;
                case 0x0A: { 
                    bool keyPressed = false;
                    for (int i = 0; i < 16; ++i) {
                        if (keypad[i]) { registers[x] = i; keyPressed = true; break; }
                    }
                    if (!keyPressed) pc -= 2; 
                } break;
                case 0x15: delayTimer = registers[x]; break;
                case 0x18: soundTimer = registers[x]; break;
                case 0x1E: index += registers[x]; break;
                case 0x29: index = FONTSET_START_ADDRESS + (registers[x] * 5); break;
                case 0x33:
                    memory[index] = registers[x] / 100;
                    memory[index + 1] = (registers[x] / 10) % 10;
                    memory[index + 2] = registers[x] % 10;
                    break;
                case 0x55: for (int i = 0; i <= x; ++i) memory[index + i] = registers[i]; break;
                case 0x65: for (int i = 0; i <= x; ++i) registers[i] = memory[index + i]; break;
            }
            break;
    }
}

void Chip8::UpdateTimers() {
    if (delayTimer > 0) --delayTimer;
    if (soundTimer > 0) --soundTimer;
}
