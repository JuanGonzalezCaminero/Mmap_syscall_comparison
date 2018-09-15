#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <string.h>

int get_file_size(FILE* file){
	//Moves the position indicator to the end of the file and 
	//gets the position
	fseek(file, 0L, SEEK_END);
	int file_size = ftell(file);
	//Resets the position to the start of the file
	rewind(file);
	return file_size;
}

long get_elapsed_time(struct timespec start_time, struct timespec end_time){
	long seconds = (long)(end_time.tv_sec - start_time.tv_sec);
	long nanoseconds;

	//The first and last seconds were not complete
	if(seconds >= 1){
		seconds -= 1;
		nanoseconds = 1000000000 - start_time.tv_nsec +
			end_time.tv_nsec + seconds * 1000000000;
	}else{
		nanoseconds = end_time.tv_nsec - start_time.tv_nsec;
	}

	return nanoseconds;
}

/*This program opens a file and loads it into memory in chunks of n bytes,
its first read using system calls (fread function), after that the file
is mapped into memory using mmap and its contents copied into the buffer
in chunks of n bytes*/
void main(int argc, char *argv){
	struct timespec start_time;
	struct timespec end_time;
	long file_size;
	char *file_name;
	//The bytes read from the files will be stored in this buffer
	char *read_buffer;
	FILE *file;
	char *output_file_name = "output_mmap_average.csv";
	FILE *output_file;
	char *buffer_str;
	//The file will be read in parts of buffer_size bytes
	long buffer_size = 4;
	long nanoseconds;
	//The average reading time is extracted from n experiments
	long number_of_repeats = 10;
	//Accumulated time
	long total_time;
	//Average time
	long average_time;

	//Kindly asking the kernel to allocate a few
	//bytes for my char array
	file_name = malloc(50 * sizeof(char));

	buffer_str = malloc(50 * sizeof(char));

	if(file_name == NULL || buffer_str == NULL){
		printf("Se ha producido un error al reservar memoria\n");
		return;
	}

	//Getting the file name from the user
	printf("File name: ");
	scanf("%s", file_name);
	file = fopen(file_name, "r");
	if(file == NULL){
		printf("Se ha producido un error al abrir el fichero\n");
		return;
	}

	output_file = fopen(output_file_name, "w");
	if(output_file == NULL){
		printf("Se ha producido un error al crear el fichero de salida\n");
		return;
	}

	//Getting the size of the file
	file_size = get_file_size(file);

	sprintf(buffer_str, "File size:, %ld Bytes", file_size);
	//fputs writes to the file but, unlike with fwrite, it writes 
	//al the characters in the string by default, without having to indicate
	//size and number of elements
	fputs(buffer_str, output_file);
	fwrite("\n", 1, 1, output_file);

	sprintf(buffer_str, "Repeats per buffer size:, %ld", number_of_repeats);
	fputs(buffer_str, output_file);
	fwrite("\n", 1, 1, output_file);

	fwrite("Buffer size, Mean elapsed time\n", 31, 1, output_file);

	//I'm lazy to initialize the buffer string, I'll just allocate more memory
	buffer_str = malloc(50 * sizeof(char));
	
	//Mapping the file to virtual memory:

	//Pointer to the beggining of the mapping
	char *mapping_start;
	char *read_pointer;

	//Getting the file descriptor for the file
	int fd = fileno(file);

	printf("Reading the file using memory mapping\n");

	for(buffer_size = 4; buffer_size <= file_size; buffer_size *= 2){
		total_time = 0;
		//Move this to an "allocate_buffer_space function"
		read_buffer = malloc(buffer_size * sizeof(char));

		if(read_buffer == NULL){
			printf("Se ha producido un error al reservar memoria\n");
			return;
		}

		for(int n = 0; n < number_of_repeats; n++){

			//I remap the file for each read so as not to have the results altered
			//by cached pages and data that is still in memory
			mapping_start = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
			read_pointer = mapping_start;
			//Are all bytes initialized with the contents of the file, since I 
			//used the length of the file as the length of the mapping? the manual
			//seems to say this is the case

			//Getting the start time
			clock_gettime(CLOCK_MONOTONIC, &start_time);
			//Reading the file
			//I copy the contents from the mapped locations into the buffer char 
			//array. I do this because its what is done before using system calls,
			//taking n bytes of data from the file and copying them to a location
			//in memory. The information will be duplicated in memory in this case,
			//but the functionality is the same
			for(int i = 0; i < file_size; i += buffer_size){
				strncpy(read_buffer, read_pointer, buffer_size);
				read_pointer += buffer_size * sizeof(char); 
			}

			//Gets the end time
			clock_gettime(CLOCK_MONOTONIC, &end_time);

			//Calculating the elapsed time between those points
			nanoseconds = get_elapsed_time(start_time, end_time);

			printf("Buffer size: %ld\n", buffer_size);
			printf("Elapsed time: %ldns (%ld.%ld seconds)\n",
				nanoseconds,
				nanoseconds / 1000000000,
				nanoseconds % 1000000000);
			printf("Start: %ld, %ld\n", start_time.tv_sec, start_time.tv_nsec);
			printf("End: %ld, %ld\n", end_time.tv_sec, end_time.tv_nsec);

			total_time += nanoseconds;

			fflush(stdout);
		}

		//Integer division
		average_time = total_time/number_of_repeats;

		sprintf(buffer_str, "%ld, %ld", buffer_size, average_time);
		fwrite(buffer_str, 50, 1, output_file);
		fwrite("\n", 1, 1, output_file);
	}
	
}

