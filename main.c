// This is just a user land test script before we make the module

#include<sys/mman.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


#define GPIO_BASE_ADDR (0x3F000000 + 0x00200000) // peripheral base + gpio offset
#define	GPIO_PERI_BASE_2711 (0xFE000000 + 0x00200000)
#define	GPIO_PERI_BASE_OLD  (0x20000000 + 0x00200000)
#define	PAGE_SIZE		(4*1024)
#define	BLOCK_SIZE		(4*1024)
#define LED_PIN 5

#define GPFSEL0 GPIO_BASE_ADDR
#define FSEL5_SHIFT 15

// offsets here: https://github.com/janne/bcm2835/blob/master/bcm2835.h
// datasheet: https://datasheets.raspberrypi.com/bcm2835/bcm2835-peripherals.pdf

__uint32_t * getGPSET_offset_addr(__uint32_t * base_addr, int pin_number)
{
    if(pin_number <= 31 )
    {
        return (__uint32_t *)((char*)base_addr + 0x1c);
    }
}



// cat /sys/kernel/debug/gpio
int main()
{

    int fd = open("/dev/gpiomem", O_RDWR | O_SYNC );
    if (fd < 0)
    {
        printf("Failed to open /dev/mem\n");
        return 1;
    }

    //1024 page size
    __uint32_t *gpio_registers = (__uint32_t *)mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE_ADDR);
    if(gpio_registers < 0)
    {
        printf("Failed to mem map\n");
        return 1;
    }
    close(fd);
    printf("Successfully mem mapped addr %p.\n", gpio_registers);
    printf("Successfully mem mapped addr %s.\n", (char *)gpio_registers);

    // __uint32_t fselect_index = LED_PIN/10;
    // __uint32_t fselect_bit_position = LED_PIN%10;
    // __uint32_t gpio_fselect = gpio_registers + fselect_index;

    // __uint32_t gpio_on_reg = (__uint32_t *)((char *)gpio_registers + 0x1c);

    *gpio_registers |= (1<<15); // turn pin 5 into output // gpio_registers + GPFSEL0 offset // GPFSEL0 -> FSEL5 (for pin 5) -> datasheet says pin 5 is bits 17-15 in word GPFSEL0
    __uint32_t *gpset_word = (__uint32_t *)((char *)gpio_registers + 0x1c);
    __uint32_t *gpclr_word = (__uint32_t *)((char *)gpio_registers + 0x28);
    for(int i=0; i<30; i++)
    {
        printf("%d\n", i);
        *gpset_word |= (1<<5);
        sleep(1);
        *gpclr_word |= (1<<5);
        sleep(1);
    }
    if(munmap((void *)gpio_registers, BLOCK_SIZE) < 0)
    {
        printf("Failed to cleanup\n");
        return 1;
    }
    return 0;
}