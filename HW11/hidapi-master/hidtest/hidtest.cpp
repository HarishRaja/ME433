#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "hidapi.h"
FILE *ofp;

#define MAX_STR 255

int main(int argc, char* argv[])
{
    int res;
    unsigned char buf[65];
    char message [128];
    wchar_t wstr[MAX_STR];
    hid_device *handle;
    int i, acx,acy,acz,aczfilter,aczaverage;
    int j = 0;
    int rownum = 0;
    

    
    // Initialize the hidapi library
    res = hid_init();
    
    // Open the device using the VID, PID,
    // and optionally the Serial number.
    handle = hid_open(0x4d8, 0x3f, NULL);
    
    // Read the Manufacturer String
    res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
    wprintf(L"Manufacturer String: %s\n", wstr);
    
    // Read the Product String
    res = hid_get_product_string(handle, wstr, MAX_STR);
    wprintf(L"Product String: %s\n", wstr);
    
    // Read the Serial Number String
    res = hid_get_serial_number_string(handle, wstr, MAX_STR);
    wprintf(L"Serial Number String: (%d) %s\n", wstr[0], wstr);
    
    // Read Indexed String 1
    res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
    wprintf(L"Indexed String 1: %s\n", wstr);
    
    // Toggle LED (cmd 0x80). The first byte is the report number (0x0).
    buf[0] = 0x0;
    buf[1] = 0x0;
    
    scanf("%d", &rownum);
    
    buf[2] = rownum;
    
    scanf("%s", message);
    
    for(i = 0; i<25; i++){
        buf[i+3] = message[i];
    }
    
    res = hid_write(handle, buf, 65);
    
    ofp = fopen("accelsHW11.txt", "w");
    
    // Request state (cmd 0x81). The first byte is the report number (0x0).
    while(j<500)
    {
        buf[0] = 0x0;
        buf[1] = 0x1;
        res = hid_write(handle, buf, 65);
    
        // Read requested state
        res = hid_read(handle, buf, 65);
        
        if (buf[1]==1) {
            // Print out the returned buffer.
            if(j % 2 == 0){
                acz = ~(buf[2] | (buf[3]<<8))+1;
                aczfilter = ~(buf[4] | (buf[5]<<8))+1;
                aczaverage = ~(buf[6] | (buf[7]<<8))+1;
                
                fprintf(ofp,"%d %d\r\n",acz,aczfilter,aczaverage);
                printf("%d\n",j);
            }
            
        
            j++;
        }
    }
    
    fclose(ofp);
    
    // Finalize the hidapi library
    res = hid_exit();
    
    return 0;
}