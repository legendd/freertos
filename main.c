#define USE_STDPERIPH_DRIVER
#include "stm32f10x.h"
#include "stm32_p103.h"
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>

/* Filesystem includes */
#include "filesystem.h"
#include "fio.h"

extern const char _sromfs;

static void setup_hardware();

volatile xSemaphoreHandle serial_tx_wait_sem = NULL;
volatile xSemaphoreHandle serial_rx_queue = NULL;
char ch;

/* IRQ handler to handle USART2 interruptss (both transmit and receive
 * interrupts). */
void USART2_IRQHandler()
{
	static signed portBASE_TYPE xHigherPriorityTaskWoken;

	/* If this interrupt is for a transmit... */
	if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
		/* "give" the serial_tx_wait_sem semaphore to notfiy processes
		 * that the buffer has a spot free for the next byte.
		 */
		xSemaphoreGiveFromISR(serial_tx_wait_sem, &xHigherPriorityTaskWoken);

		/* Diables the transmit interrupt. */
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		/* If this interrupt is for a receive... */
	}
	else if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
		ch = USART_ReceiveData(USART2); 
		if(!xQueueSendToBackFromISR(serial_rx_queue, &ch, &xHigherPriorityTaskWoken))
			while(1);
	}
	else {
		/* Only transmit and receive interrupts should be enabled.
		 * If this is another type of interrupt, freeze.
		 */
		while(1);
	}

	if (xHigherPriorityTaskWoken) {
		taskYIELD();
	}
}

void send_byte(char ch)
{
	/* Wait until the RS232 port can receive another byte (this semaphore
	 * is "given" by the RS232 port interrupt when the buffer has room for
	 * another byte.
	 */
	while (!xSemaphoreTake(serial_tx_wait_sem, portMAX_DELAY));

	/* Send the byte and enable the transmit interrupt (it is disabled by
	 * the interrupt).
	 */
	USART_SendData(USART2, ch);
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

char recv_byte(){
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	char msg;
	/*xQueueReceive & xQueueCreate are defined in queue.h
	portBASE_TYPE xQueueReceive(
	                            xQueueHandle xQueue,
                               void *pvBuffer,
                               portTickType xTicksToWait
                            );*/
	while(!xQueueReceive(serial_rx_queue, &msg, portMAX_DELAY));
	return msg;
}
/*
void read_romfs_task(void *pvParameters)
{
	char buf[128];
	size_t count;
	int fd = fs_open("/romfs/test.txt", 0, O_RDONLY);
	do {
		//Read from /romfs/test.txt to buffer
		count = fio_read(fd, buf, sizeof(buf));
		
		//Write buffer to fd 1 (stdout, through uart)
		fio_write(1, buf, count);
	} while (count);
	
	while (1);
}*/
void shell_task(void *pvParameters)
{
	char str_temp[21] = "Implement Shell Task";
	char buf[128];
	while(1){
		fio_write(1,str_temp,sizeof(str_temp));
		int count = fio_read(0, buf, 127);
	}
}
int main()
{
	init_rs232();
	enable_rs232_interrupts();
	enable_rs232();
	
	fs_init();
	fio_init();
	
	register_romfs("romfs", &_sromfs);
	
	/* Create the queue used by the serial task.  Messages for write to
	 * the RS232. */
	vSemaphoreCreateBinary(serial_tx_wait_sem);

	serial_rx_queue = xQueueCreate(1, sizeof(ch));

	/* Create a task to output text read from romfs. */
//	xTaskCreate(read_romfs_task,
//	            (signed portCHAR *) "Read romfs",
//	            512 /* stack size */, NULL, tskIDLE_PRIORITY + 2, NULL);

	xTaskCreate(shell_task,
	            (signed portCHAR *) "Implement Shell Task",
	            512 /* stack size */, NULL, tskIDLE_PRIORITY + 2, NULL);
	/* Start running the tasks. */
	vTaskStartScheduler();

	return 0;
}

int str_to_int(char *str)
{
	int i=0,tmp=0;
	while(str[i]!='\0')
	{
		/*using ascii code to do this comparison*/
		if (str[i]>='0'&&str[i]<='9') tmp = tmp*10 + (str[i]-'0');
		else return -1;
		i++;
	}
	return tmp;
}
void itoa(int n, char *buffer){
	if (n == 0)
		*(buffer++) = '0';
	else {		
		int a=10000;
		if (n < 0) {
			*(buffer++) = '-';
			n = -n;
		}
		while (a!=0) {
			int i = n / a;
			if (i != 0) {
				*(buffer++) = '0'+(i%10);;
		}
		a/=10;
	}
}
	*buffer = '\0';
}
void vApplicationTickHook()
{
}
