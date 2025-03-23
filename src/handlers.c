#include "pic.h"

#define MAX_QUEUE_SIZE 16

typedef struct {
    uint8_t items[MAX_QUEUE_SIZE];
    int front;
    int rear;
} Queue;

Queue scancode_queue = {{0}, 0, 0};

void enqueue(uint8_t scancode) {
	if (scancode_queue.front != scancode_queue.rear && scancode_queue.items[scancode_queue.front] == scancode) {
		// If the scancode is already at the front, don't enqueue it again
		return;
	}
    if ((scancode_queue.rear + 1) % MAX_QUEUE_SIZE != scancode_queue.front) { // Check if queue is not full
        scancode_queue.items[scancode_queue.rear] = scancode;
        scancode_queue.rear = (scancode_queue.rear + 1) % MAX_QUEUE_SIZE;
    }
}

uint8_t dequeue() {
    if (scancode_queue.front != scancode_queue.rear) { // Check if queue is not empty
        uint8_t scancode = scancode_queue.items[scancode_queue.front];
        scancode_queue.front = (scancode_queue.front + 1) % MAX_QUEUE_SIZE;
        return scancode;
    }
    return 0;  // Return 0 if the queue is empty
}

int queue_empty() {
    return scancode_queue.front == scancode_queue.rear;
}

void keyboard_handler(void)
{
    uint8_t scancode = inb(0x60); // Read from the keyboard data port (0x60)

    enqueue(scancode);

    // Send EOI to PIC after handling IRQ1 (keyboard interrupt)
    PIC_sendEOI(1);
}
