void kernel_main(void) 
{

	char *video_memory = (char *) 0xB8000;
    const char *str = "Meyaba Kendi Kernel'im calisiyor!";
    
    int i = 0;
    while (str[i] != '\0') {
        video_memory[i * 2] = str[i];
        video_memory[i * 2 + 1] = 0x07;
        i++;
    }
    
    while(1);

}
