#ifndef CHIP8_HPP
#define CHIP8_HPP

#include <cstdint>
#include <random>

// Hardware Constants
const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_START_ADDRESS = 0x50;
const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;

// Aesthetic Customization
const uint32_t COLOR_BACKGROUND = 0x0A100DFF; // Deep Arcade Slate
const uint32_t COLOR_FOREGROUND = 0x33FF33FF; // Retro Phosphor Green

class Chip8 {
public:
    uint8_t registers[16]{};
    uint8_t memory[4096]{};
    uint16_t index{};
    uint16_t pc{};
    uint16_t stack[16]{};
    uint8_t sp{};
    uint8_t delayTimer{};
    uint8_t soundTimer{};
    uint8_t keypad[16]{};
    uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};
    uint16_t opcode{};

    std::default_random_engine randGen;
    std::uniform_int_distribution<int> randByte;

    Chip8();
    void LoadROM(char const* filename);
    void Cycle();
    void UpdateTimers();
};

#endif
