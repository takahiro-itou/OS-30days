
void api_putchar(int c);
void api_end(void);

void HariMain(void)
{
    volatile char a[100];
    a[10] = 'A';        /*  これはもちろんいい  */
    api_putchar(a[10]);
    a[102] = 'B';       /*  これはまずいよね。  */
    api_putchar(a[102]);
    a[123] = 'C';       /*  これもまずいよね。  */
    api_putchar(a[123]);
    a[133] = 'D';
    api_putchar(a[133]);
    api_end();
}
