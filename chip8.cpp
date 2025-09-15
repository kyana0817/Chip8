#include "chip8.hpp"
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>

void chip8::initialize()
{
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;

    for (int i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];
};

void chip8::loadROM(const std::string &path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open ROM");
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> buffer(size);
    if (file.read(reinterpret_cast<char *>(buffer.data()), size))
    {
        for (size_t i = 0; i < buffer.size(); ++i)
        {
            memory[0x200 + i] = buffer[i];
        }
    }
    else
    {
        throw std::runtime_error("Failed to read ROM");
    }
};

void chip8::emulateCycle()
{
    opcode = memory[pc] << 8 | memory[pc + 1];
    switch (opcode & 0xF000)
    {
    case 0x0000:
        switch (opcode & 0x000f)
        {
        case 0x0000:
            break;
        case 0x000E:
            break;
        default:
            printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
        }
    case 0x2000:
        stack[sp] = pc;
        ++sp;
        pc = opcode & 0x0FFF;
        break;
    case 0x8000:
        switch (opcode & 0x000F)
        {
        case 0x0004:
        {
            if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                V[0xF] = 1;
            else
                V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        }
        default:
            printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
        }
    case 0xA000:
        I = opcode & 0x0FFF;
        pc += 2;
        break;
    case 0xD000:
    {
        unsigned short x = V[(opcode & 0x0F00) >> 8];
        unsigned short y = V[(opcode & 0x00F0) >> 4];
        unsigned short height = opcode & 0x000F;
        unsigned short pixel;

        V[0xF] = 0;
        for (int yline = 0; yline < height; yline++)
        {
            pixel = memory[I + yline];
            for (int xline = 0; xline < 8; xline++)
            {
                if ((pixel & (0x80 >> xline)) != 0)
                {
                    if (gfx[(x + xline + ((y + yline) * 64))] == 1)
                        V[0xF] = 1;
                    gfx[x + xline + ((y + yline) * 64)] ^= 1;
                }
            }
        }
        drawFlag = true;
        pc += 2;
    }
    break;
    case 0xE000:
        switch (opcode & 0x00FF)
        {
        case 0x009E:
            if (key[V[(opcode & 0x0F00) >> 8]] != 0)
                pc += 4;
            else
                pc += 2;
            break;
        default:
            printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
        }
    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x0033:
            memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
            pc += 2;
            break;
        default:
            printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
        }
    default:
        printf("Unknown opcode: 0x%X\n", opcode);
    }
    if (delay_timer > 0)
        --delay_timer;
    if (sound_timer > 0)
    {
        if (sound_timer == 1)
            printf("BEEP!\n");
        --sound_timer;
    }
};
