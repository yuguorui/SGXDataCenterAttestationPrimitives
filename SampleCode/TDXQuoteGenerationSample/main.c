#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include "tdx_attest.h"

#define HEX_DUMP_SIZE	16

static void print_hex_dump(const char *title, const char *prefix_str,
			   const uint8_t *buf, int len)
{
	const uint8_t *ptr = buf;
	int i, rowsize = HEX_DUMP_SIZE;

	if (!len || !buf)
	return;

	fprintf(stdout, "\t\t%s", title);

	for (i = 0; i < len; i++) {
		if (!(i % rowsize))
			fprintf(stdout, "\n%s%.8x:", prefix_str, i);
		if (ptr[i] <= 0x0f)
			fprintf(stdout, " 0%x", ptr[i]);
			else
			fprintf(stdout, " %x", ptr[i]);
	}

	fprintf(stdout, "\n");
}

void gen_report_data(uint8_t *reportdata)
{
	int i;

	srand(time(NULL));

	for (i = 0; i < TDX_REPORT_DATA_SIZE; i++)
		reportdata[i] = rand();
}

int main(int argc, char *argv[])
{
	uint32_t quote_size = 0;
	tdx_report_data_t report_data = {{0}};
	tdx_report_t tdx_report = {{0}};
	tdx_uuid_t selected_att_key_id = {0};
	uint8_t *p_quote_buf = NULL;
	FILE *fptr = NULL;

	int opt;
	int provided_report_data = 0;
	while ((opt = getopt(argc, argv, "d:")) != -1) {
		switch (opt) {
			case 'd': 
			{
				int str_len = strlen(optarg);
				provided_report_data = 1;

				if (str_len % 2) {
					fprintf(stderr, "Report data size is not even\n");
					return 1;
				}

				if (str_len / 2 > TDX_REPORT_DATA_SIZE) {
					fprintf(stderr, "Report data size is too large\n");
					return 1;
				}

				for (int i = 0; i < str_len; i++) {
					if (!isxdigit(optarg[i])) {
						fprintf(stderr, "Invalid character in report data\n");
						return 1;
					}
				}

				memset(report_data.d, 0, TDX_REPORT_DATA_SIZE);
				for (int i = 0; i < str_len; i += 2) {
					char byte[3] = {optarg[i], optarg[i + 1], '\0'};
					report_data.d[i / 2] = strtol(byte, NULL, 16);
				}
				break;
			}
			default:
				fprintf(stderr, "Usage: %s [-d <report data>]\n", argv[0]);
				return 1;
		}
	}

	if (!provided_report_data)
		gen_report_data(report_data.d);

	print_hex_dump("\n\t\tTDX report data\n", " ", report_data.d, sizeof(report_data.d));

	if (TDX_ATTEST_SUCCESS != tdx_att_get_report(&report_data, &tdx_report)) {
		fprintf(stderr, "\nFailed to get the report\n");
		return 1;
	}
	fptr = fopen("report.dat","wb");
	if( fptr ) {
		fwrite(&tdx_report, sizeof(tdx_report), 1, fptr);
		fclose(fptr);
	}
	fprintf(stdout, "\nWrote TD Report to report.dat\n");

	if (TDX_ATTEST_SUCCESS != tdx_att_get_quote(&report_data, NULL, 0, &selected_att_key_id,
					     &p_quote_buf, &quote_size, 0)) {
		fprintf(stderr, "\nFailed to get the quote\n");
		return 1;
	}
	print_hex_dump("\n\t\tTDX quote data\n", " ", p_quote_buf, quote_size);

	fprintf(stdout, "\nSuccessfully get the TD Quote\n");
	fptr = fopen("quote.dat","wb");
	if( fptr )
	{
		fwrite(p_quote_buf, quote_size, 1, fptr);
		fclose(fptr);
	}
	fprintf(stdout, "\nWrote TD Quote to quote.dat\n");

	tdx_att_free_quote(p_quote_buf);
	return 0;
}

