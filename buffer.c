#include "buffer.h"


/*
 * Name : void Buffer_Reset(buffer_t *buffer)
 *
 * Purpose : This function initialize or reset the buffer's structure. The counter
 *           of characters is clear, input and output pointer are set to the start of the buffer.
 *
 * Input : buffer, pointer on buffer_t structure
 *
 * Output : void
 *
 * History :
 *   04-04-2005 : Creation - Pierrick Calvet
 */
void Buffer_Reset(buffer_t *buffer)
{
	buffer->count = 0;					// No chars 
	buffer->p_in = buffer->tab;		// Input and output pointers
	buffer->p_out = buffer->tab;		// at the beginning of array
}


/*
 * Name : char Buffer_Push(buffer_t *buffer, unsigned char byte)
 *
 * Purpose : Push a byte into the buffer
 *
 * Inputs : buffer, pointer on buffer_t structure
 *          byte, byte to put into the buffer
 *
 * Outputs : 0 if no error occurs
 *          -1 if the buffer is full
 *
 * History :
 *   04-04-2005 : Creation - Pierrick Calvet
 */
char Buffer_Push(buffer_t *buffer, unsigned char byte)
{
	if(buffer->count == BUFFER_SIZE)								// Return error
		return -1;														// if buffer full

	*(buffer->p_in) = byte;											// Push byte in the buffer
	buffer->count++;													// Increment number of bytes in the buffer

	if(++(buffer->p_in) == (buffer->tab + BUFFER_SIZE))	// If input pointer at the end of buffer,
		buffer->p_in = buffer->tab;								// set it at the start

	return 0;															// Success
}


/*
 * Name : char Buffer_Pull(buffer_t *buffer, unsigned char *byte)
 *
 * Purpose : Pull a byte from the buffer. The byte is put at address pointed by *byte.
 *
 * Inputs : buffer, pointer on buffer_t structure
 *          byte, adress of a var where the byte pull from the buffer will be put.
 *
 * Outputs : 0 if no error occurs
 *          -1 if the buffer is empty
 *
 * History :
 *   04-04-2005 : Creation - Pierrick Calvet
 */
char Buffer_Pull(buffer_t *buffer, unsigned char *byte)
{
	if(buffer->count == 0)											// Return error
		return -1;														// if buffer empty

	*byte = *(buffer->p_out);										// Return character where are the pointer
	buffer->count--;													// Decrement number of characters in the buffer

	if (++(buffer->p_out) == (buffer->tab + BUFFER_SIZE))	// If output pointer at the end of buffer,
		buffer->p_out = buffer->tab;								// set it at the start

	return 0;															// Success
}
