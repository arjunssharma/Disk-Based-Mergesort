/*
 ============================================================================
 Name        : Disk-Based.c
 Author      : Arjun Sharma
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>

int comparator_keys(const void *key1, const void *key2);
int generate_sorted_runs(char *index_file_name, int *output_index, int *run_count);
void basic_merge_sort(char* index_file_name, char* sorted_index_file_name);
void multistep_merge_sort(char* index_file_name, char* sorted_index_file_name);
void heapify(int elements_count, int position);
void swap(int *input_buffer, int index, int size);
void merge(int runs_count_for_super_runs, char *index_file_name, char *intermediate_sorted_file_name, int extension);
void replacement_selection_merge_sort(char* index_file_name, char* sorted_index_file_name);
void print_time(struct timeval start_time,struct timeval end_time);


struct timeval start_time, end_time;
#define input_buffer_size 1000
int input_buffer[input_buffer_size];
#define output_buffer_size 1000
int output_buffer[output_buffer_size];

int main(int argc, char* argv[]) {
	if (argc != 4) {
		printf("Illegal Number of Arguments");
		exit(0);
	}


	char *indexFileName = argv[2];
	char *sortedIndexFileName = argv[3];


	gettimeofday(&start_time, NULL);

	if (strcmp(argv[1], "--basic") == 0) {
		basic_merge_sort(indexFileName, sortedIndexFileName);
	} else if (strcmp(argv[1], "--multistep") == 0) {
		multistep_merge_sort(indexFileName, sortedIndexFileName);
	} else if (strcmp(argv[1], "--replacement") == 0) {
		replacement_selection_merge_sort(indexFileName, sortedIndexFileName);
	} else {
		printf("Illegal Order");
		exit(0);
	}

	gettimeofday(&end_time, NULL);
	print_time(start_time, end_time);

	return 0;
}

void print_time(struct timeval start_time,struct timeval end_time)
{
	double total_time = (((double)end_time.tv_sec + 1.0e-6*end_time.tv_usec) - ((double)start_time.tv_sec + 1.0e-6*start_time.tv_usec) );
	printf( "Time: %0.6f\n", total_time);
}

int get_minimum_position() {
	int i = 0;
	int min = INT_MAX;
	int min_position = 0;
	int buffer_size = 1000;
	for (i = 0; i < buffer_size; i++) {
		if (min >= input_buffer[i]) {
			min = input_buffer[i];
			min_position = i;
		}
	}
	return min_position;
}


void heapify(int elements_count, int position) {
	int i = elements_count / 2;
	while (i >= position) {
		swap(input_buffer + position, i, elements_count);
		i--;
	}
}

void swap(int *input_buffer, int index, int size){
	int small = 0;
	int left = 0;
	int right = 0;
	int top = input_buffer[index];

	while (index < (size/2)) {
		left = 2*index + 1;
		right = 2*index + 2;

		if ((right < size) &&  (input_buffer[right] < input_buffer[left]))
			small = right;
		else
			small = left;

		if (input_buffer[small] >= top)
			break;

		input_buffer[index] = input_buffer[small];
		index = small;
	}

	input_buffer[index] = top;
}


void basic_merge_sort(char* index_file_name, char* sorted_index_file_name) {
	FILE *file_input = fopen(index_file_name, "r+b");
	fseek(file_input, 0L, SEEK_END);
	int total_keys = ftell(file_input) / sizeof(int);
	fseek(file_input, 0L, SEEK_SET);
	FILE *file_output;
	FILE *file_temporary;
	int buffer_runs = 0;
    int i = 0;
	int minimum_value = INT_MAX;

	if (total_keys < 1000) {
		char buffer[5] = "";
		char intermediate_file_name[50] = "";
		strcat(intermediate_file_name, index_file_name);
		strcat(intermediate_file_name, ".");
		sprintf(buffer, "%0.03d", i);
		strcat(intermediate_file_name, buffer);
		fread(input_buffer, total_keys, sizeof(int), file_input);
		qsort(input_buffer, total_keys, sizeof(int), comparator_keys);
		file_temporary = fopen(intermediate_file_name, "w+b");
		file_output = fopen(sorted_index_file_name, "w+b");
		fwrite(input_buffer, sizeof(int), total_keys, file_temporary);
		fwrite(input_buffer, sizeof(int), total_keys, file_output);
		fclose(file_temporary);
	} else {
		int to_be_read = total_keys / input_buffer_size;
		FILE **runs = malloc(to_be_read * sizeof(FILE*));
		for (i = 0; i < to_be_read; i++) {
			char buffer[5] = "";
			char intermediate_file_name[50] = "";
			strcat(intermediate_file_name, index_file_name);
			strcat(intermediate_file_name, ".");
			sprintf(buffer, "%0.03d", i);
			strcat(intermediate_file_name, buffer);
			fread(input_buffer, input_buffer_size, sizeof(int), file_input);
			qsort(input_buffer, input_buffer_size, sizeof(int), comparator_keys);
			runs[i] = fopen(intermediate_file_name, "w+b");
			fwrite(input_buffer, sizeof(int), input_buffer_size, runs[i]);
			fseek(runs[i], 0L, SEEK_SET);
		}
		fclose(file_input);

		//merge logic
		buffer_runs = input_buffer_size / to_be_read;
		for (i = 0; i < to_be_read; i++) {
			fread(input_buffer + (i * buffer_runs), buffer_runs, sizeof(int), runs[i]);
		}

		file_output = fopen(sorted_index_file_name, "w+b");
		int output_index = 0;
		int position = 0;
		while(1) {
			position = get_minimum_position();
			minimum_value = input_buffer[position];
			//break condition
			if (minimum_value == INT_MAX) {
				fwrite(output_buffer, sizeof(int), output_index, file_output);
				break;
			}

			output_buffer[output_index] = input_buffer[position];
			output_index++;
			input_buffer[position] = INT_MAX; //reset

			if (output_index == output_buffer_size) {
				fwrite(output_buffer, sizeof(int), output_buffer_size, file_output);
				output_index = 0;
			}

			if ((position + 1) % buffer_runs == 0) {
				int run_value = position / buffer_runs;
				fread(input_buffer + (run_value * buffer_runs), buffer_runs, sizeof(int), runs[run_value]);
			}
		}

		//close runs
		for (i = 0; i < to_be_read; i++) {
			fclose(runs[i]);
		}
	}
	fclose(file_output);
}

int comparator_keys(const void *key1, const void *key2) {
	return (*(int*) key1 - *(int*) key2);
}

void replacement_selection_merge_sort(char* index_file_name, char* sorted_index_file_name) {
	FILE *file_input = fopen(index_file_name, "r+b");
	FILE *file_output_sorted = fopen(sorted_index_file_name, "w+b");
	int primary_heap_index = 0;
	int secondary_heap_index = 749;
	int output_buffer_write = 0;
	char intermediate_file_name[50] = "";

	int number_of_entries = 750;
	int heap_size = fread(input_buffer, sizeof(int), number_of_entries, file_input);
	if (heap_size == 0 || (file_output_sorted == NULL)) {
		exit (0);
	}

	int read_input_buffer = fread(input_buffer + number_of_entries, sizeof(int), (input_buffer_size - number_of_entries), file_input);
	int read_buffer = 0;
	read_buffer += read_input_buffer;
	read_buffer += heap_size;
	//printf("heap size: %d", heap_size);
	heapify(heap_size, 0);
	int runs_count = 0;
	snprintf(intermediate_file_name, (2 * input_buffer_size), "%s.%03d", index_file_name, runs_count);
	FILE *file_temporary = fopen(intermediate_file_name, "w+b");
	int output_index = 0;
	int read_count = 0;
	int tmp_primary_index = 0;
	int tmp_secondary_index = 0;
	int heap_size_secondary = 0;
	while (1) {
		if (heap_size > 0) { //initially 750
			if (read_input_buffer > 0) {
				output_buffer[output_index] = (primary_heap_index != 0) ? input_buffer[tmp_primary_index]: input_buffer[primary_heap_index];
				read_count++;
				if (primary_heap_index == 0 && input_buffer[number_of_entries] >= input_buffer[primary_heap_index]) {
					input_buffer[primary_heap_index] = input_buffer[number_of_entries];
					number_of_entries++;
					swap(input_buffer, primary_heap_index, (heap_size));

				} else if (primary_heap_index == 749 && input_buffer[number_of_entries] >= input_buffer[tmp_primary_index]) {
					input_buffer[tmp_primary_index] = input_buffer[number_of_entries];
					number_of_entries++;
					swap(input_buffer, tmp_primary_index, heap_size);
				} else {
					if (heap_size > 0) {
						if (primary_heap_index == 0)
							input_buffer[primary_heap_index] = input_buffer[(primary_heap_index + heap_size - 1)];
						else
							input_buffer[tmp_primary_index] = input_buffer[(tmp_primary_index + heap_size - 1)];
					}

					heap_size = heap_size - 1;

					if (secondary_heap_index != 0)
						input_buffer[(secondary_heap_index - heap_size_secondary)] = input_buffer[number_of_entries];
					else
						input_buffer[(tmp_secondary_index - heap_size_secondary)] = input_buffer[number_of_entries];

					heap_size_secondary++;
					number_of_entries++;

					if (primary_heap_index == 0)
						swap(input_buffer, primary_heap_index, heap_size);
					else
						swap(input_buffer, tmp_primary_index, heap_size);
				}
				output_index++;
			}

			if (output_index == output_buffer_size) {
				fwrite(output_buffer, sizeof(int), output_index, file_temporary);
				output_buffer_write += output_buffer_size;
				output_index = 0;
			}

			if (number_of_entries == (read_input_buffer + 750)) {
				number_of_entries = 750;
				read_input_buffer = fread(input_buffer + number_of_entries, sizeof(int), (input_buffer_size - number_of_entries), file_input);
				read_buffer += read_input_buffer;
			}

			if (read_input_buffer <= 0) {
				if (output_index > 0) {
					fwrite(output_buffer, sizeof(int), output_index, file_temporary);
					output_buffer_write += output_index;
					fclose(file_temporary);
					output_index = 0;
					runs_count++;
					//issue here for run 166 & 167 -- file size not matching with provided output
					snprintf(intermediate_file_name, (2 * input_buffer_size), "%s.%03d", index_file_name, runs_count);
					file_temporary = fopen(intermediate_file_name, "w+b");
				}

				if (heap_size > 0) {
					if (primary_heap_index == 0)
						qsort(input_buffer, heap_size, sizeof(int), comparator_keys);
					else
						qsort((input_buffer + (tmp_primary_index)), heap_size, sizeof(int), comparator_keys);
				}

				if (heap_size_secondary > 0) {
					if (secondary_heap_index == 0)
						qsort((input_buffer + (tmp_secondary_index - heap_size_secondary + 1)), heap_size_secondary, sizeof(int), comparator_keys);
					else
						qsort((input_buffer + (secondary_heap_index - heap_size_secondary + 1)), heap_size_secondary, sizeof(int), comparator_keys);
				}

				int tmp_prim_index = 0;
				int tmp_second_index = 0;

				if (primary_heap_index == 0) {
					tmp_prim_index = 0;
					tmp_second_index = 749;
				} else {
					tmp_prim_index = 749;
					tmp_second_index = 0;
				}

				while (heap_size > 0 || heap_size_secondary > 0) {
					if (heap_size > 0 && heap_size_secondary > 0) {
						if (tmp_prim_index == 0) {
							if (input_buffer[primary_heap_index] < input_buffer[(secondary_heap_index - heap_size_secondary + 1)]) {
								output_buffer[output_index] = input_buffer[primary_heap_index];
								output_index++;
								primary_heap_index++;
								heap_size--;
							} else {
								output_buffer[output_index] = input_buffer[(secondary_heap_index - heap_size_secondary + 1)];
								output_index++;
								heap_size_secondary--;
							}
						} else {
							if (input_buffer[(tmp_secondary_index - heap_size_secondary + 1)] < input_buffer[tmp_primary_index]) {
								output_buffer[output_index] = input_buffer[(tmp_secondary_index - heap_size_secondary + 1)];
								output_index++;
								heap_size_secondary--;
							} else {
								output_buffer[output_index] = input_buffer[tmp_primary_index];
								output_index++;
								tmp_primary_index++;
								heap_size--;
							}
						}

					} else if (heap_size > 0) {
						if (tmp_prim_index == 0) {
							output_buffer[output_index] = input_buffer[primary_heap_index];
							output_index++;
							primary_heap_index++;
							heap_size--;
						} else {
							output_buffer[output_index] = input_buffer[tmp_primary_index];
							output_index++;
							tmp_primary_index++;
							heap_size--;
						}

					} else if (heap_size_secondary > 0) {
						if (tmp_second_index != 0) {
							output_buffer[output_index] = input_buffer[(secondary_heap_index - heap_size_secondary + 1)];
							output_index++;
							heap_size_secondary--;
						} else {
							output_buffer[output_index] = input_buffer[(tmp_secondary_index - heap_size_secondary + 1)];
							output_index++;
							heap_size_secondary--;
						}
					}
				}

				if (output_index > 0) {
					fwrite(output_buffer, sizeof(int), output_index, file_temporary);
					output_buffer_write += output_index;
					output_index = 0;
				}

				fclose(file_temporary);
				break;
			}
		} else {
			runs_count++;
			if (output_index > 0) {
				fwrite(output_buffer, sizeof(int), output_index, file_temporary);
				output_buffer_write += output_index;
				output_index = 0;
			}

			fclose(file_temporary);
			snprintf(intermediate_file_name, (2 * input_buffer_size), "%s.%03d", index_file_name, runs_count);
			file_temporary = fopen(intermediate_file_name, "w+b");

			if (secondary_heap_index == 749)
				tmp_primary_index = (secondary_heap_index - heap_size_secondary + 1);
			else
				tmp_primary_index = 0;

			if (primary_heap_index == 0)
				tmp_secondary_index = 749;
			else
				tmp_secondary_index = 0;

			int temp = heap_size;
			heap_size = heap_size_secondary;
			heap_size_secondary = temp;
			temp = primary_heap_index;
			primary_heap_index = secondary_heap_index;
			secondary_heap_index = temp;

			if (primary_heap_index == 749)
				heapify(heap_size, primary_heap_index - heap_size + 1);
			else
				heapify(heap_size, 0);
		}
	}

	runs_count += 1;
	merge(runs_count, index_file_name, sorted_index_file_name, 0);
}



void multistep_merge_sort(char* index_file_name, char* sorted_index_file_name)
{
	int i=0;
	int output_index = 0;
	int runs_count_to_produce_super_runs = 15;
	int run_count = 0;
	int return_code = generate_sorted_runs(index_file_name, &output_index, &run_count);

	if(return_code == 0 || run_count == 0)
	{
		exit(0);
	}

	if( run_count <= runs_count_to_produce_super_runs )
		runs_count_to_produce_super_runs = run_count;

	int super_runs_count = 0;
	if((run_count % runs_count_to_produce_super_runs ) != 0)
		super_runs_count = (run_count / runs_count_to_produce_super_runs) + 1;
	else
		super_runs_count = run_count / runs_count_to_produce_super_runs;

	char intermediate_file_name[50] = "";
	char super_intermediate_file_name[50] = "";
	for( i = 0; i< super_runs_count; i++ )
	{
		snprintf(intermediate_file_name, (2 * input_buffer_size), "%s.super.%03d",index_file_name,i);
	    if((runs_count_to_produce_super_runs*(i + 1)) < run_count)
	    {
	    	merge( runs_count_to_produce_super_runs, index_file_name, intermediate_file_name, (runs_count_to_produce_super_runs * i));
	    }
	    else
	    {
	    	merge((run_count - (runs_count_to_produce_super_runs*i) ), index_file_name, intermediate_file_name, (runs_count_to_produce_super_runs * i));
	    }

	}

	snprintf(super_intermediate_file_name, (2 * input_buffer_size), "%s.super",index_file_name);
	merge(super_runs_count, super_intermediate_file_name, sorted_index_file_name, 0);
}

void merge(int runs_count_for_super_runs, char *index_file_name, char *intermediate_sorted_file_name, int extension)
{
	FILE *file_output = fopen(intermediate_sorted_file_name, "w+b");
	FILE *runs[runs_count_for_super_runs];
	int i = 0;
	int output_index = 0;
	int runs_exhausted = 0;
	int actualBufferReadByEachRun[runs_count_for_super_runs], min = 0;
	int runs_buffer[runs_count_for_super_runs];
	char intermediate_file_name[50] = "";
	int buffer_size_for_run = input_buffer_size / runs_count_for_super_runs;
	for (i = 0; i < runs_count_for_super_runs; i++) {
		snprintf(intermediate_file_name, (2 * output_buffer_size), "%s.%03d", index_file_name, (i + extension));
		runs[i] = fopen(intermediate_file_name, "r+b");
		actualBufferReadByEachRun[i] = fread((input_buffer + (i * buffer_size_for_run)), sizeof(int), buffer_size_for_run, runs[i]);
		runs_buffer[i] = 0;
	}

	int min_position = 0;
	while (1) {
		min = INT_MAX;
		for (i = 0; i < runs_count_for_super_runs; i++) {
			if (runs_buffer[i] != -1) {
				if (min >= input_buffer[(runs_buffer[i]) + (buffer_size_for_run * i)]) {
					min = input_buffer[(runs_buffer[i]) + (buffer_size_for_run * i)];
					min_position = i;
				}
			}
		}

		output_buffer[output_index] = min;
		output_index++;
		runs_buffer[min_position] += 1;

		if (runs_buffer[min_position] == actualBufferReadByEachRun[min_position] && actualBufferReadByEachRun[min_position] > 0) {
			if ((actualBufferReadByEachRun[min_position] = fread((input_buffer + (min_position * buffer_size_for_run)), sizeof(int), buffer_size_for_run, runs[min_position])) != 0) {
				runs_buffer[min_position] = 0;
			} else {
				runs_buffer[min_position] = -1;
			}
		}

		if (output_index == output_buffer_size) {
			fwrite(output_buffer, sizeof(int), output_index, file_output);
			output_index = 0;
		}

		for (i = 0; i < runs_count_for_super_runs; i++) {
			if (runs_buffer[i] != -1) {
				runs_exhausted = 1;
				break;
			} else
				runs_exhausted = 0;
		}

		if (runs_exhausted == 0) {
			if (output_index > 0) {
				fwrite(output_buffer, sizeof(int), output_index, file_output);
			}
			break;
		}
	}

	for (i = 0; i < runs_count_for_super_runs; i++) {
		fclose(runs[i]);
	}
	fclose(file_output);
}




int generate_sorted_runs(char *index_file_name, int *output_index, int *run_count)
{
	FILE *file_input = fopen(index_file_name, "r+b");
	FILE *sorted_run_file = NULL;
	int read_input = 0;
	char intermediate_file_name[50] = "";
	int value = 0;

	while((read_input = fread(input_buffer, sizeof(int), input_buffer_size, file_input)) != 0) {
		snprintf(intermediate_file_name, (2 * input_buffer_size), "%s.%03d", index_file_name, value);
		sorted_run_file = fopen(intermediate_file_name, "w+b");
		if (read_input < input_buffer_size) {
			qsort(input_buffer, read_input, sizeof(int), comparator_keys);
			fwrite(input_buffer, sizeof(int), read_input, sorted_run_file);
			*output_index = read_input;
		} else {
			qsort(input_buffer, input_buffer_size, sizeof(int), comparator_keys);
			fwrite(input_buffer, sizeof(int), input_buffer_size, sorted_run_file);
			*output_index = input_buffer_size;
		}

		fclose(sorted_run_file);
		value++;
	}

	*run_count = value;
	fclose(file_input);
	return (1);
}
