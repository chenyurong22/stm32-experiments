#define RCC_AHB1ENR *((volatile unsigned int *)0x40023830)

#define GPIOA_MODER *((volatile unsigned int *)0x40020000)
#define GPIOD_MODER *((volatile unsigned int *)0x40020C00)

#define GPIOA_IDR *((volatile unsigned int *)0x40020010)
#define GPIOD_ODR *((volatile unsigned int *)0x40020C14)

int main(void)
{
  // activate clock for port a (0) and port d (3)
  RCC_AHB1ENR |= (1 << 0) | (1 << 3);

  // led (pin 12) as output (01)
  GPIOD_MODER &= ~(3 << 24);
  GPIOD_MODER |= (1 << 24);

  // button (pin 0) as input (00)
  GPIOA_MODER &= ~(3 << 0);

  while (1)
  {
    if (GPIOA_IDR & (1 << 0))
    {
      GPIOD_ODR |= (1 << 12);
    }
    else
    {
      GPIOD_ODR &= ~(1 << 12);
    }
  }
}