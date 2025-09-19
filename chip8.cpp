#include "chip8.hpp"
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdlib>

void Chip8::initialize()
{
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;

    for (int i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];
};

void Chip8::loadROM(const std::string &path)
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

void Chip8::emulateCycle()
{
    opcode = memory[pc] << 8 | memory[pc + 1];
    switch (opcode & 0xF000)
    {
    case 0x0000:
        switch (opcode & 0x00FF)
        {
        case 0xE0:
            std::fill(std::begin(gfx), std::end(gfx), 0);
            drawFlag = true;
            pc += 2;
            break;
        case 0xEE:
            pc = stack[--sp];
            pc += 2;
            break;
        default:
            printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
            break;
        }
        break;
    case 0x1000:
        pc = opcode & 0x0FFF;
        break;
    case 0x2000:
        stack[sp++] = pc;
        pc = opcode & 0x0FFF;
        break;
    case 0x3000:
        if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
        {
            pc += 4;
        }
        else
        {
            pc += 2;
        }
        break;
    case 0x4000:
        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
        {
            pc += 4;
        }
        else
        {
            pc += 2;
        }
        break;
    case 0x5000:
        if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
        {
            pc += 4;
        }
        else
        {
            pc += 2;
        }
        break;
    case 0x6000:
        V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        pc += 2;
        break;
    case 0x7000:
        V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        pc += 2;
        break;
    case 0x8000:
        switch (opcode & 0x000F)
        {
        case 0x0000:
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0001:
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0002:
            V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0003:
            V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
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
        case 0x0005:
        {
            if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                V[0xF] = 0;
            else
                V[0xF] = 1;
            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        }
        case 0x0006:
            V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
            V[(opcode & 0x0F00) >> 8] >>= 1;
            pc += 2;
            break;
        case 0x0007:
        {
            if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
                V[0xF] = 0;
            else
                V[0xF] = 1;
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        }
        case 0x000E:
            V[0xF] = (V[(opcode & 0x0F00) >> 8] & 0x80) >> 7;
            V[(opcode & 0x0F00) >> 8] <<= 1;
            pc += 2;
            break;
        default:
            printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
            break;
        }
        break;
    case 0x9000:
        if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
        {
            pc += 4;
        }
        else
        {
            pc += 2;
        }
        break;
    case 0xA000:
        I = opcode & 0x0FFF;
        pc += 2;
        break;
    case 0xB000:
        pc = (opcode & 0x0FFF) + V[0];
        break;
    case 0xC000:
        V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
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
        case 0x00A1:
            if (key[V[(opcode & 0x0F00) >> 8]] == 0)
                pc += 4;
            else
                pc += 2;
            break;
        default:
            printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
            break;
        }
        break;
    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x0007:
            V[(opcode & 0x0F00) >> 8] = delay_timer;
            pc += 2;
            break;
        case 0x000A:
        {
            auto len = sizeof(key) / sizeof(key[0]);
            for (int i = 0; i < len; ++i)
            {
                if (key[i] != 0)
                {
                    V[(opcode & 0x0F00) >> 8] = i;
                    keyPressFlag = true;
                }
            }
            if (!keyPressFlag)
                return;
            pc += 2;
            break;
        }
        case 0x0015:
            delay_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        case 0x0018:
            sound_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        case 0x001E:
            if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
                V[0xF] = 1;
            else
                V[0xF] = 0;
            I += V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        case 0x0029:
            I = V[(opcode & 0x0F00) >> 8] * 5;
            pc += 2;
            break;
        case 0x0033:
            memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
            pc += 2;
            break;
        case 0x0055:
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                memory[I + i] = V[i];
            I += ((opcode & 0x0F00) >> 8) + 1;
            pc += 2;
            break;
        case 0x0065:
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                V[i] = memory[I + i];
            I += ((opcode & 0x0F00) >> 8) + 1;
            pc += 2;
            break;
        default:
            printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
            break;
        }
        break;
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
