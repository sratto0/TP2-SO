// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include <keyboard.h>
#include <lib.h>
#include <semaphore.h>
#include <stdint.h>
#include <time.h>
#include <video.h>

#define BUFFER_CAPACITY 256
#define HOTKEY 29
#define CTRL_D_SCANCODE 0x20 // Scancode para 'd' sin modificar
#define CTRL_C_SCANCODE 0x2E // Scancode para 'c' sin modificar
#define LSHIFT_SCANCODE 0x2A        // <-- Shift izq
#define RSHIFT_SCANCODE 0x36        // <-- Shift der
#define ASCII_EOF 0x04
#define KEYBOARD_SEM_ID "999" // ID único para el semáforo del teclado
#define KBD_EOF_MARKER 0xFF

static uint8_t ctrl_pressed = 0;
static uint8_t shift_pressed = 0;
static uint8_t _bufferStart = 0;
static uint16_t _bufferSize = 0;
static uint8_t _buffer[BUFFER_CAPACITY] = {0};

extern void kill_foreground_process();

static const char charHexMap[256] = {
    0,   0,    '1',  '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b', ' ',  'q', 'w',  'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',  '\n', 0,   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', 0,    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',  0,    '*', 0,    ' ', 0,   0,   0,   0,   0,   0
};

static const char charHexMapShift[256] = {
    0,   0,   '!','@','#','$','%','^','&','*','(',')','_','+','\b',' ',
    'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,'A','S',
    'D','F','G','H','J','K','L',':','"', 0,  0,  '|', 'Z','X','C','V','B','N','M','<','>','?',0,'*',0,' ',
    0,0,0,0,0,0
};

static int getBufferIndex(int offset) {
  return (_bufferStart + offset) % BUFFER_CAPACITY;
}

void keyboard_init() {
  // Crear semáforo inicializado en 0 (no hay teclas disponibles)
  my_sem_open(KEYBOARD_SEM_ID, 0);
}

void keyboardHandler() {
  uint8_t key = getKeyPressed();

  // Release
  if (key & 0x80) {
    uint8_t code = key & 0x7F;
    if (code == HOTKEY) ctrl_pressed = 0;
    if (code == LSHIFT_SCANCODE || code == RSHIFT_SCANCODE) shift_pressed = 0;  // <-- soltar Shift
    return;
  }

  // Press
  if (key == HOTKEY) { ctrl_pressed = 1; saveRegisters(); return; }
  if (key == LSHIFT_SCANCODE || key == RSHIFT_SCANCODE) { shift_pressed = 1; return; } // <-- presionar Shift

  if (ctrl_pressed && key == CTRL_C_SCANCODE) { kill_foreground_process(); return; }

  if (ctrl_pressed && key == CTRL_D_SCANCODE) {
    if (_bufferSize < BUFFER_CAPACITY) {
      _buffer[getBufferIndex(_bufferSize)] = KBD_EOF_MARKER;
      _bufferSize++;
      my_sem_post(KEYBOARD_SEM_ID);
    }
    return;
  }

  if (_bufferSize < BUFFER_CAPACITY) {
    _buffer[getBufferIndex(_bufferSize)] = key;   // encolamos scancode
    _bufferSize++;
    my_sem_post(KEYBOARD_SEM_ID);
  }
}

// void keyboardHandler() {
//   uint8_t key = getKeyPressed();

//   // Detectar Ctrl press/release
//   if (key == HOTKEY) {
//     ctrl_pressed = 1;
//     saveRegisters();
//     return;
//   }
//   if (key == (HOTKEY | 0x80)) { // Ctrl release
//     ctrl_pressed = 0;
//     return;
//   }

//   if (!(key & 0x80)) { // Solo teclas presionadas
//     if (ctrl_pressed && key == CTRL_C_SCANCODE) {
//       kill_foreground_process();
//       return;
//     }
//     // Ctrl+D = EOF
//     if (ctrl_pressed && key == CTRL_D_SCANCODE) {
//       if (_bufferSize < BUFFER_CAPACITY) {
//         _buffer[getBufferIndex(_bufferSize)] = KBD_EOF_MARKER; // ASCII EOF
//         _bufferSize++;
//         my_sem_post(KEYBOARD_SEM_ID);
//       }
//       return;
//     }

//     if (_bufferSize < BUFFER_CAPACITY) {
//       _buffer[getBufferIndex(_bufferSize)] = key;
//       _bufferSize++;
//       my_sem_post(KEYBOARD_SEM_ID);
//     }
//   }
// }

char getScancode() {
  // Wait: esperar a que haya una tecla disponible (bloquea si buffer vacío)
  my_sem_wait(KEYBOARD_SEM_ID);

  // Aquí ya hay al menos una tecla garantizada
  char c = _buffer[_bufferStart];
  _bufferStart = getBufferIndex(1);
  _bufferSize--;

  return c;
}

int getAscii() {
  char scancode = getScancode();
  if ((unsigned char)scancode == KBD_EOF_MARKER) { // EOF
    return ASCII_EOF; // O 0x04, según cómo manejes EOF
  }
  const char *map = shift_pressed ? charHexMapShift : charHexMap;
  return map[(unsigned char)scancode];
}
