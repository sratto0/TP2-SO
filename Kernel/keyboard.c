// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include <keyboard.h>
#include <stdint.h>
#include <lib.h>
#include <video.h>
#include <time.h>
#include <semaphore.h>

#define BUFFER_CAPACITY 256
#define HOTKEY 29
#define KEYBOARD_SEM_ID 999  // ID único para el semáforo del teclado

static uint8_t _bufferStart = 0;
static uint16_t _bufferSize = 0;
static uint8_t _buffer[BUFFER_CAPACITY] = {0};

static const char charHexMap[256] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', ' ',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 0, 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0
};

static int getBufferIndex(int offset){
    return (_bufferStart + offset) % BUFFER_CAPACITY;
}

void keyboard_init() {
    // Crear semáforo inicializado en 0 (no hay teclas disponibles)
    my_sem_open(KEYBOARD_SEM_ID, 0);
}

void keyboardHandler(){
    uint8_t key = getKeyPressed();
    
    if (!(key & 0x80)) {  // Solo teclas presionadas (no releases)
        if (key == HOTKEY) {
            saveRegisters();
            return;
        }
        
        if (_bufferSize < BUFFER_CAPACITY) {
            _buffer[getBufferIndex(_bufferSize)] = key;
            _bufferSize++;
            
            // Signal: hay una tecla disponible
            my_sem_post(KEYBOARD_SEM_ID);
        }
    }
}

char getScancode() {
    // Wait: esperar a que haya una tecla disponible (bloquea si buffer vacío)
    my_sem_wait(KEYBOARD_SEM_ID);
    
    // Aquí ya hay al menos una tecla garantizada
    char c = _buffer[_bufferStart];
    _bufferStart = getBufferIndex(1);
    _bufferSize--;
    
    return c;
}

char getAscii(){
    return charHexMap[(int) getScancode()];
}