// NASDAQ ITCH 5.0 parser
// Reference: Nasdaq TotalView-ITCH 5.0 Specification
// http://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <libgen.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#if defined(__linux__)
#include <byteswap.h>
#else /* macOS */

// Some pre-defined binary-text helpers:
// http://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf
#define bswap_16(value) \
((((value) & 0xff) << 8) | ((value) >> 8))

#define bswap_32(value) \
(((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | \
(uint32_t)bswap_16((uint16_t)((value) >> 16)))

#define bswap_64(value) \
(((uint64_t)bswap_32((uint32_t)((value) & 0xffffffff)) << 32) | \
(uint64_t)bswap_32((uint32_t)((value) >> 32)))
#endif

#define parse_uint16(a) \
bswap_16(*((uint16_t *)(a)))

#define parse_uint32(a) \
bswap_32(*((uint32_t *)(a)))

#define parse_uint64(a) \
bswap_64(*((uint64_t *)(a)))

#define parse_uint16(a) \
bswap_16(*((uint16_t *)(a)))

#define parse_ts(a) \
(((uint64_t)bswap_16(*((uint16_t *)(a)))) << 32) | \
(uint64_t)bswap_32(*(uint32_t *)((a)+2))

#define parse_stock(n) \
for (i = 0; i < 8; i++) { \
  if (m[i+(n)] == ' ') { \
    stock[i] = 0; \
    break; \
  } else { \
    stock[i] = m[i+(n)]; \
  } \
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s input_file_path output_folder_path \n\n", argv[0]);
    exit(1);
  }

  unsigned char msg_type[22] = {'S', 'R', 'H', 'Y', 'L', 'V', 'W', 'K', 'J', 
    'h', 'A', 'F', 'E', 'C', 'X', 'D', 'U', 'P', 'Q', 'B', 'I', 'N'};

  bool parse_flag[128];
  int i, j;
  for (i = 0; i < 128; i++) {
    parse_flag[i] = false;
  }

  if (argc == 3) {
    for (i = 0; i < 22; i++) {
      if (msg_type[i] == 'P') { // Only parsing the 'P' type message.
        parse_flag[msg_type[i]] = true;
      }
    }
  }

  bool found;
  unsigned char c;

  // argv[1]: input file path
  FILE *f_input = fopen(argv[1], "r");
  if (f_input == NULL) {
    fprintf(stderr, "Error opening file %s: %s\n", argv[1], strerror(errno));
    exit(1);
  }

  char *argv1, *input_file_name, *output_base;
  argv1 = strdup(argv[1]);
  input_file_name = basename(argv1);
  output_base = strdup(input_file_name);

  for (unsigned long i=0; i < strlen(input_file_name); i++) {
    if (output_base[i] == '.') {
      output_base[i] = 0;
      break;
    }
  }

  // argv[2]: output folder path
  int rv = mkdir(argv[2], 0755);
  if (rv == -1) {
    if (errno == EEXIST) {
      fprintf(stderr, "Warning: output directory %s already exists!\n", argv[2]);
    }
    else {
      fprintf(stderr, "Error making directory %s: %s\n", argv[2], strerror(errno));
      exit(1);
    }
  }

  // total number of all messages
  unsigned long total = 0;
  // total number of messages for each message type
  unsigned long total_type[22];
  for (i = 0; i < 22; i++) {
    total_type[i] = 0;
  }

  time_t start =  time(NULL);

  printf("=========== Parsing Nasdaq ITCH v5.0 Begins ===========\n");
  printf("Nasdaq Source File: %s\n", input_file_name);
  printf("Result Outputs to Folder: %s\n", argv[2]);

  FILE *file_output[22];
  char csv_filename[32];
  char csv_full_path[256];

  // open scv files only for specified message types
  for (i = 0; i < 22; i++) {
    unsigned char t = msg_type[i];
    file_output[i] = NULL;
    if (parse_flag[t]) {
      sprintf(csv_filename, "%s-%c.csv", output_base, t);
      printf("Output file: %s\n", csv_filename);
      sprintf(csv_full_path, "%s/%s", argv[2], csv_filename);
      file_output[i] =  fopen(csv_full_path, "w");
      if (file_output[i] == NULL) {
        fprintf(stderr, "Error opening file %s: %s\n", csv_full_path, strerror(errno));
        exit(1);
      }
    }
  }
  free(argv1);
  free(output_base);

  uint16_t msg_header;
  uint16_t msg_length;
  // message buffer
  char m[64];
  while (true) {
    // first two bytes before each message starts encodes the length of the message
    if (fread((void*)&msg_header, sizeof(char), 2, f_input) < 2) {
      // This is the EOF
      printf("=========== Parsing ITCH v5.0 ends   ===========\n");
      break;
    }
    msg_length = bswap_16(msg_header);
    if (fread((void*)m, sizeof(char), msg_length, f_input) < msg_length) {
        fprintf(stderr, "Error reading input file: %s\n", strerror(errno));
        goto panic;
    }

    // security symbol for the issue in the Nasdaq execution system
    char stock[9];
    stock[8] = 0;
    char t = m[0];
    switch (t) {
      // Parse Traded Message Only
      case 'P':
        if (parse_flag['P']) {
          // Parsing the binary: Please see reference Nasdaq Totalview Itch 5.0.
          uint16_t stock_locate = parse_uint16(m + 1);
          uint16_t tracking_number = parse_uint16(m + 3);
          uint64_t timestamp = parse_ts(m + 5);
          uint64_t order_reference_number = parse_uint64(m + 11);
          uint32_t shares = parse_uint32(m + 20);
          parse_stock(24)
          uint32_t price = parse_uint32(m + 32);
          uint64_t match_number = parse_uint64(m + 36);
          // Write to CSV:
          fprintf(file_output[17],
            "%c,%u,%u,%llu.%09llu,%llu,%c,%u,%s,%u.%04u,%llu\n",
            t, stock_locate, tracking_number,
            timestamp/1000000000, timestamp%1000000000,
            order_reference_number, m[19], shares, stock,
            price/10000, price%10000, match_number);
          total_type[17]++;
          total++;
        }
        break;

    }
  }

  // Display Parsing Information.
  printf("Total number of all messages parsed: %lu\n", total);
  for (i = 0; i < 22; i++) {
    if (msg_type[i] == 'P') {
      printf("Total number of %c messages parsed: %lu\n", msg_type[i], total_type[i]);
    }
  }

  time_t end = time(NULL);
  printf("Time spent: %ld seconds\n", (end - start));

  fclose(f_input);
  for (i = 0; i < 22; i++) {
    unsigned char t = msg_type[i];
    if (parse_flag[t]) fclose(file_output[i]);
  }
  return 0;

// In case something went wrong, we close the file descriptor.
panic:
  fclose(f_input);
  for (i = 0; i < 22; i++) {
    unsigned char t = msg_type[i];
    if (parse_flag[t]) fclose(file_output[i]);
  }
  exit(1);
}
