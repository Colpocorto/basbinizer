#include <stdio.h>
#include <malloc.h>
#include <string.h>


#define LOADER_SIZE 174
#define PATCH_POS 51
#define PATCH_NAME_POS  0x12
#define CAS_NAME_LEN    6

unsigned char loader_data[LOADER_SIZE] = {
    0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74, 0xD0, 0xD0, 0xD0, 0xD0,
    0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0x47, 0x41, 0x4D, 0x45, 0x20, 0x20,
    0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74, 0x75, 0xF9, 0xF4, 0xF9,
    0x75, 0xF9, 0xF3, 0x3E, 0xC9, 0x32, 0x9A, 0xFD, 0x32, 0x9F, 0xFD, 0x21,
    0x00, 0x80, 0x01, 0x00, 0x00, 0xCD, 0xB1, 0xF9, 0xF3, 0x22, 0xC2, 0xF6,
    0x21, 0x01, 0x80, 0x22, 0x76, 0xF6, 0x2B, 0x22, 0x48, 0xFC, 0x3E, 0x0F,
    0x21, 0xE9, 0xF3, 0x77, 0x3E, 0x01, 0x23, 0x77, 0x23, 0x77, 0xCD, 0x62,
    0x00, 0xCD, 0x6C, 0x00, 0xAF, 0x32, 0x00, 0x80, 0xC3, 0xAC, 0x73, 0xC3,
    0x00, 0x40, 0xF3, 0xC5, 0xE5, 0x3A, 0x07, 0x00, 0x3C, 0x32, 0xEF, 0xF9,
    0x32, 0xF3, 0xF9, 0xCD, 0xE1, 0x00, 0x38, 0xFB, 0xCD, 0xE4, 0x00, 0x38,
    0xFB, 0xE1, 0x77, 0x23, 0x3A, 0xEC, 0xF9, 0xEE, 0x06, 0x32, 0xEC, 0xF9,
    0xCD, 0xED, 0xF9, 0xC1, 0x0B, 0x79, 0xB0, 0xC5, 0xE5, 0x20, 0xE5, 0xE1,
    0xC1, 0xCD, 0xE7, 0x00, 0xAF, 0xCD, 0xF3, 0x00, 0x3E, 0x01, 0xC3, 0xED,
    0xF9, 0x06, 0xF3, 0xD3, 0x99, 0x3E, 0x87, 0xD3, 0x99, 0xC9, 0x1F, 0xA6,
    0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74};

void usage(void);
void title(void);

void title() {
    fprintf(stdout, "_/_/_/_/_/_/_/_/  Basbinizer v0.1 _/_/_/_/_/_/_/_/\n");
    fprintf(stdout, "_/_/_/_/_/_/_/_/ 2023 MSXWiki.org _/_/_/_/_/_/_/_/\n\n\n");
    
}
void usage() {
    fprintf(stdout,"Syntax:\n\n");
    fprintf(stdout,"basbinizer <inputfile> <outputfile> <CAS: FILENAME>\n\n");
    fprintf(stdout,"Where intputfile is the path to a MSX-BASIC .BAS file in tokenized format (ASCII files not supported yet) and outputfile is the resulting .CAS file. The name of the BLOAD name is given by \"<CAS: FILENAME> (max. 6 characters)\"\n\n");
    fprintf(stdout,"Example:\tbasbinizer STARS.BAS STARS.CAS STARS\n\n\n");

}
int main(int argc, char **argv) {
    
    title();
    
    int exitcode = 0;

    FILE    *fi = NULL;
    FILE    *fo = NULL;

    char    *fi_path = NULL;
    char    *fo_path = NULL;

    if (argc < 4) {
        usage();
        return (exitcode);
    }

    fi_path = argv[1];

    fprintf(stdout,"Opening file %s...\t", fi_path);

    //try to open input file

    fi = fopen(fi_path, "rb");

    if (!fi) {
        fprintf(stdout,"FAIL\n\n");
        fprintf(stdout,"Could not open input file. Exiting...\n\n");
        exitcode = 2;

        return(exitcode);

    }
    fprintf(stdout, "OK\n");

    //Get file size
    fseek(fi, 0L, SEEK_END);
    int file_size = ftell(fi);
    rewind(fi);

    //Write file size into binary loader data
    loader_data[PATCH_POS] =    (unsigned char)(file_size & 0xff);
    loader_data[PATCH_POS+1] =  (unsigned char)((file_size & 0xff00)>>8);

    int name_len = strlen(argv[3]);

    memset(loader_data+PATCH_NAME_POS, ' ', CAS_NAME_LEN);
    memcpy(loader_data+PATCH_NAME_POS, argv[3], name_len>CAS_NAME_LEN ? CAS_NAME_LEN : name_len);
 
    //Get memory for the buffer
    unsigned char *buffer = malloc(LOADER_SIZE + file_size);

    if (!buffer) {
        fprintf(stdout,"Memory error trying to find %d bytes free... exiting.\n\n",LOADER_SIZE + file_size);
        exitcode = 1;

        return(exitcode);
    }

    //copy data and concat .BAS file
    memcpy(buffer,loader_data,LOADER_SIZE);
    int read_data = fread(buffer+LOADER_SIZE, 1, file_size, fi);
    if (read_data < (file_size)) {
        fprintf(stdout,"Memory error while reading input file... Exiting\n\n");
        exitcode = 1;
        
        return(exitcode);
    }

    //try to open input file
    fo_path = argv[2];
    
    fprintf(stdout,"Opening file %s...\t", fo_path);
    fo = fopen(fo_path, "wb");

    if (!fo) {
        fprintf(stdout,"FAIL\n\n");
        fprintf(stdout,"Could not open input file. Exiting...\n\n");
        exitcode = 2;

        return(exitcode);

    }
    fprintf(stdout, "OK\n");    

    //write data to output file
    int written = fwrite(buffer, 1, file_size + LOADER_SIZE, fo);
    if (written < file_size+LOADER_SIZE) {
        fprintf(stdout,"Memory error while writing output file... Exiting\n\n");
        exitcode = 1;
        
        return(exitcode);        
    }
    

    //close input file
    fprintf(stdout,"Closing file %s...\t", fi_path);
    fclose(fi);
    fprintf(stdout,"OK\n");

    //close output file
    fprintf(stdout,"Closing file %s...\t", fo_path);
    fclose(fo);
    fprintf(stdout,"OK\n");    

    //free resources
    free(buffer);


    return (exitcode);
}

