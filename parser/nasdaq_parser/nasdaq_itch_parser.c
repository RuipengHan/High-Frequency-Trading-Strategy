// Copyright [2022] <Ruipeng Han>
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
#include <inttypes.h>
#if defined(__linux__)
#include <byteswap.h>
#else /* macOS */

// Some pre-defined binary-text helpers:
// http://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf
// http://tradingphysics.com/Resources/Specifications/HistoricalItch.aspx
#define bswap_16(value) \
((((value) & 0xff) << 8) | ((value) >> 8))

#define bswap_32(value) \
(((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | \
(uint32_t)bswap_16((uint16_t)((value) >> 16)))

#define bswap_64(value) \
(((uint64_t)bswap_32((uint32_t)((value) & 0xffffffff)) << 32) | \
(uint64_t)bswap_32((uint32_t)((value) >> 32)))
#endif

#define parse_uint32(a) \
bswap_32(*((uint32_t *)(a)))

#define parse_uint64(a) \
bswap_64(*((uint64_t *)(a)))

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
const char tick_type = 'T';
const char *market_center = "NASDAQ";
const int num_msg_types = 22;
unsigned char msg_type[] = {'S', 'R', 'H', 'Y', 'L', 'V', 'W',
'K', 'J', 'h', 'A', 'F', 'E', 'C', 'X', 'D', 'U', 'P', 'Q', 'B', 'I', 'N'};

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s INPUT_FILE OUTPUT_PATH SYMBOL\n", argv[0]);
    fprintf(stderr, "You must provide the path for the source Nasdaq file, "
     "the output directory the program writes to, "
     "and the company symbol you want to parse.");
    fprintf(stderr, "NOTE: Your Nasdaq source file must be named "
    "in the default convention: MMDDYYYY.NASDAQ_ITCH50 \n\n");
    exit(1);
  }
  char *target_symbol = strdup(argv[3]);
  bool parse_flag[128];
  int i;
  for (i = 0; i < 128; i++) {
    parse_flag[i] = false;
  }

  for (i = 0; i < num_msg_types; i++) {
    // Only parsing the 'P' type message, so this program outputs only one csv.
    if (msg_type[i] == 'P') {
      parse_flag[msg_type[i]] = true;
    }
  }

  // argv[1]: input file path
  FILE *f_input = fopen(argv[1], "r");
  if (f_input == NULL) {
    fprintf(stderr, "Error opening file %s: %s\n", argv[1], strerror(errno));
    exit(1);
  }

  /* Extract the date of data and setup the output file name using date. */
  char *argv1, *input_file_name, *output_base;
  char year[5]; year[4] = 0;
  char month[3]; month[2] = 0;
  char day[3]; day[2] = 0;
  argv1 = strdup(argv[1]);
  input_file_name = basename(argv1);
  output_base = strdup(input_file_name);
  strncpy(year, output_base+4, 4);
  strncpy(month, output_base, 2);
  strncpy(day, output_base+2, 2);
  for (uint64_t i=0; i < strlen(input_file_name); i++) {
    if (output_base[i] == '.') { /* Parse the dot in MMDDYYYY.NASDAQ_ITCH50 */
      output_base[i] = 0;
      break;
    }
  }
  printf("The date of stock data being parsed: %s-%s-%s\n", year, month, day);

  // Make the output directory at the argument output path.
  int rv = mkdir(argv[2], 0755);
  if (rv == -1) {
    if (errno == EEXIST) {
      fprintf(stderr, "Warning: output directory %s already exists!\n",
      argv[2]);
    } else {
      fprintf(stderr, "Error making directory %s: %s\n", argv[2],
      strerror(errno));
      exit(1);
    }
  }

  // total number of all messages
  int total = 0;
  // total number of messages for each message type
  int total_type[22];
  for (i = 0; i < num_msg_types; i++) {
    total_type[i] = 0;
  }

  time_t start =  time(NULL);

  printf("=========== Parsing Nasdaq ITCH v5.0 Begins ===========\n");
  printf("Nasdaq Source File: %s\n", input_file_name);
  printf("Result Outputs to Folder: %s\n", argv[2]);

  FILE *file_output[num_msg_types];
  char csv_filename[32];
  char csv_full_path[256];

  // open csv files only for specified message types
  for (i = 0; i < num_msg_types; i++) {
    unsigned char t = msg_type[i];
    file_output[i] = NULL;
    if (parse_flag[t]) {
      snprintf(csv_filename, sizeof(csv_filename), "tick_%s_%s%s%s.txt",
      target_symbol, year, month, day);
      printf("Output file: %s\n", csv_filename);
      snprintf(csv_full_path, sizeof(csv_full_path), "%s/%s",
      argv[2], csv_filename);
      file_output[i] =  fopen(csv_full_path, "w");
      if (file_output[i] == NULL) {
        fprintf(stderr, "Error opening file %s: %s\n", csv_full_path,
        strerror(errno));
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
  // int limit = 0;
  while (true) {
    // first two bytes of each message starts encodes the length of the message.
    if (fread((void*)&msg_header, sizeof(char), 2, f_input) < 2) {
      // This is the EOF
      printf("=========== Parsing ITCH v5.0 ends   ===========\n");
      break;
    }
    msg_length = bswap_16(msg_header);
    if (fread((void*)m, sizeof(char), msg_length, f_input) < msg_length) {
        fprintf(stderr, "Error reading input file: %s\n", strerror(errno));
        fclose(f_input);
        for (i = 0; i < num_msg_types; i++) {
          unsigned char t = msg_type[i];
          if (parse_flag[t]) fclose(file_output[i]);
        }
        free(target_symbol);
        exit(1);
    }

    // The scurity symbol (tick) for the issue in the Nasdaq execution system,
    // maximal length of 5.
    char stock[6];
    stock[5] = 0;
    char t = m[0];
    // if (limit > 100) { /* Generate 100 rows ONLY. */
    //   break;
    // }
    switch (t) {
      // Parse Traded Message Only
      case 'P':
        if (parse_flag['P']) {
          /* For debugging, limit the number of rows in output. */
          // if (limit < 1520000) {
          //   continue;
          // }
          // Parsing the binary: Please see reference Nasdaq Totalview Itch 5.0.

          /* Stock Symbol, right padded with spaces. */
          parse_stock(24)
          if (strcmp(stock, target_symbol)) {
            continue;
          }
          // limit += 1;
          // The UTC date time for Strategy Studio: yyyy-MM-dd HH:mm:ss.ffffff
          /* Nanoseconds since midnight. (Two parts: seconds.nanoSeconds) */
          uint64_t timestamp = parse_ts(m + 5);
          uint64_t seconds = timestamp/1000000000;
          uint64_t nano_seconds = timestamp%1000000000;
          int hour = seconds / 3600;
          /* Nasdaq data is EST, which is 4 hours eariler than UTC. */
          int utc_hour = (hour + 4) % 24;
          int minute = (seconds % 3600) / 60;
          int second = seconds - (hour * 3600 + minute * 60);
          // printf("HH:MM:SS --> %d:%d:%d", hour, minute, second);
          // printf("Stock is: %s\n", stock);
          /* The number of shares being matched in this execution. */
          uint32_t shares = parse_uint32(m + 20);

          /* The match price of the order. */
          uint32_t price = parse_uint32(m + 32);
          /* The Nasdaq generated session unique Match Number for this trade. */
          uint64_t match_number = parse_uint64(m + 36);

          // Write to CSV:
          fprintf(file_output[17],
            "%s-%s-%s %02d:%02d:%02d.%"PRIu64",%s-%s-%s"
            " %02d:%02d:%02d.%"PRIu64",%"PRIu64",%c,%s,%u.%04u,%u\n",
            year, month, day, utc_hour, minute, second, nano_seconds, year,
            month, day, utc_hour, minute, second, nano_seconds,
            match_number, tick_type, market_center, price/10000,
            price%10000, shares);
          total_type[17]++;
          total++;
        }
        break;
    }
  }

  // Display Parsing Information.
  printf("Total number of all messages parsed: %d\n", total);
  for (i = 0; i < num_msg_types; i++) {
    if (msg_type[i] == 'P') {
      printf("Total number of %c messages parsed: %d\n", msg_type[i],
      total_type[i]);
    }
  }

  time_t end = time(NULL);
  printf("Time spent: %ld seconds\n", (end - start));

  fclose(f_input);
  for (i = 0; i < num_msg_types; i++) {
    unsigned char t = msg_type[i];
    if (parse_flag[t]) fclose(file_output[i]);
  }
  free(target_symbol);
  return 0;
}
