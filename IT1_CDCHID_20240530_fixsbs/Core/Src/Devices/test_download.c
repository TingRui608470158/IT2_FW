#include "test.h"

#include "rdc_prepare_dload.h"

#if !defined(__KERNEL__) && defined(__linux__) /* Linux application */ && !defined(__ANDROID__) && !defined(ECLIPSE_RCP)
#include <term.h>  
#include <termios.h>  
#endif

#define MAX_NUM_FILENAME_BUF	256

#if defined(_WIN32) || defined(_WIN64)
	/* We are on Windows */
	#define strtok_r strtok_s
#endif

/* Full file name (inlcude path) */
static char filename_list[MAX_NUM_FILENAME_BUF][512];

#define PROGRESSVALUE(a, b) (int)((double)((a) * 100 ) / (double)(b))

/**
	Step progress bar in the below callback function or the other thread.
	Recommand: Send a message to the other thread then process
	           the progress bar at the thread.
*/

void vc_dload_proc_callback(E_VC_DLOAD_STATUS_T status,
							E_VC_LUT_CLASS_T lut_class,
							U8_T *pgm_data_buf_ptr,
							U16_T processed_item_cnt, U16_T total_item)
{
	int progress_val;
	char str_buf[128];

	switch (status) {
	case VC_DLOAD_STATUS_INPROGRESS:
		progress_val = PROGRESSVALUE(processed_item_cnt, total_item);
		if (progress_val < 100) {
			sprintf(str_buf, "Total processing... %d%% (%d/%d items)",
						progress_val, processed_item_cnt, total_item);
		}
		break;

	case VC_DLOAD_STATUS_STARTING:
		sprintf(str_buf, "Total processing... 0%% (%d/%d items)",
							0, total_item);
		time_elapse();
		break;

	case VC_DLOAD_STATUS_COMPLTED:
		sprintf(str_buf,
				"Total processing completed 100%% (%d/%d items)  %0.3f sec",
				processed_item_cnt, total_item, (float)time_elapse()/(float)1000);

		/* Free a allocated memory buffer. */
		free(pgm_data_buf_ptr);
		break;

	case VC_DLOAD_STATUS_FORCE_STOP:
		strcpy(str_buf, "Download stopped!");
		break;

	case VC_DLOAD_STATUS_DEV_FAIL:
		strcpy(str_buf, "Device interface failed!");
		break;

	case VC_DLOAD_STATUS_REMOVE_SETUP_ITEM:
		/* Free a allocated memory buffer. */
		sprintf(str_buf, "Remove setuped item. lut_class(%d)", lut_class);
		free(pgm_data_buf_ptr);
		break;

	default:
		strcpy(str_buf, "[VC_RDC] Invaild processing status\n");
		break;
	}

	DMSG("%s\n", str_buf);
}

void vc_dload_verify_callback(E_VC_DLOAD_STATUS_T status,
								E_VC_LUT_CLASS_T lut_class,
								U32_T curr_verify_addr,
								U16_T verified_bytes, U16_T total_verify_bytes)
{
	char str_buf[64];

	switch (status) {
	case VC_DLOAD_STATUS_INPROGRESS:
		DMSG(". ");
		return;

	case VC_DLOAD_STATUS_STARTING:
		sprintf(str_buf, "Verifying... 0%% (%d bytes)", total_verify_bytes);
		break;

	case VC_DLOAD_STATUS_COMPLTED:
		sprintf(str_buf, "\nVerify completed 100%% (%u/%u bytes)",
						verified_bytes, total_verify_bytes);
		break;

	case VC_DLOAD_STATUS_FORCE_STOP:
		strcpy(str_buf, "\nDownload stopped!");
		break;

	case VC_DLOAD_STATUS_DEV_FAIL:
		strcpy(str_buf, "\nDevice interface failed!");
		break;

	case VC_DLOAD_STATUS_VERIFY_ERR:
		strcpy(str_buf, "\nVerify error!");
		break;

	case VC_DLOAD_STATUS_SHUTDOWN:
		strcpy(str_buf, "\nShut down");
		break;

	default:
		strcpy(str_buf, "\nInvaild verify status\n");
		break;
	}

	DMSG("%s\n", str_buf);
}

void vc_dload_program_callback(E_VC_DLOAD_STATUS_T status,
								E_VC_LUT_CLASS_T lut_class,
								U32_T curr_pgm_addr,
								U16_T programmed_bytes, U16_T total_pgm_bytes)
{
	char str_buf[64];

	switch (status) {
	case VC_DLOAD_STATUS_INPROGRESS:
		DMSG(". ");
		return;

	case VC_DLOAD_STATUS_STARTING:
		sprintf(str_buf, "Programming... 0%% (%d bytes)", total_pgm_bytes);
		break;

	case VC_DLOAD_STATUS_COMPLTED:
		sprintf(str_buf, "\nProgram completed 100%% (%u/%u bytes)",
						programmed_bytes, total_pgm_bytes);
		break;

	case VC_DLOAD_STATUS_FORCE_STOP:
		strcpy(str_buf, "\nDownload stopped!");
		break;

	case VC_DLOAD_STATUS_DEV_FAIL:
		strcpy(str_buf, "\nDevice interface failed!");
		break;

	case VC_DLOAD_STATUS_SHUTDOWN:
		strcpy(str_buf, "\nShut down");
		break;

	default:
		strcpy(str_buf, "\n[VC_RDC] Invaild erase status\n");
		break;
	}

	DMSG("%s\n", str_buf);
}

void vc_dload_erase_callback(E_VC_DLOAD_STATUS_T status,
								E_VC_LUT_CLASS_T lut_class,
								U32_T curr_erase_addr,
								U16_T erased_sector_cnt, U16_T total_sector)
{
	char str_buf[64];

	switch (status) {
	case VC_DLOAD_STATUS_INPROGRESS:
		DMSG(". ");
		return;

	case VC_DLOAD_STATUS_STARTING:
		sprintf(str_buf, "Erasing... 0%% (%d sectors)", total_sector);
		break;

	case VC_DLOAD_STATUS_COMPLTED:
		strcpy(str_buf, "Erase completed 100%");
		break;

	case VC_DLOAD_STATUS_FORCE_STOP:
		strcpy(str_buf, "Download stopped!");
		break;

	case VC_DLOAD_STATUS_DEV_FAIL:
		strcpy(str_buf, "Device interface failed!");
		break;

	case VC_DLOAD_STATUS_SHUTDOWN:
		strcpy(str_buf, "Shut down");
		break;

#if defined(CFG_MDC_RDC100)
	case VC_DLOAD_STATUS_ERASE_SAVE:
		sprintf(str_buf,
				"Saving other data... (addr: 0x%06X, len: %d)",
				curr_erase_addr, erased_sector_cnt);
		break;

	case VC_DLOAD_STATUS_ERASE_RESTORE:
		sprintf(str_buf,
				"Restoring other data... (addr: 0x%06X, len: %d)",
				curr_erase_addr, erased_sector_cnt);
		break;
#endif

	default:
		strcpy(str_buf, "[VC_RDC] Invaild erase status\n");
		break;
	}

	DMSG("\n%s\n", str_buf);
}

static int select_download_file(int num_file)
{
	int i, sel_idx = -1;
	char read_buf[128], *token;
	int input_item_cnt = 0;

	if (num_file) {
		DMSG("=============== Selectable file list =====================\n");
		for (i = 0; i < num_file; i++)
			DMSG("    [%d]: %s\n", i, filename_list[i]);

		DMSG("========================================================\n");
		
		while (1) {
RETRY:
			DMSG("Select file (0 ~ %d), q(Quit), r(Retry):", num_file - 1);

			fgets(read_buf, 128, stdin);
		
			token = strtok(read_buf, " \n\r");
			while (token != NULL) {
				if (token[0] == 'q' || token[0] == 'Q')
					return -1000;
		
				if (token[0] == 'r' || token[0] == 'R')
					goto RETRY;
		
				if (is_string_digit(token) == FALSE) {
					EMSG("NOT digit value. Retry!\n");
					goto RETRY;
				}
		
				sel_idx = atoi(token);
				input_item_cnt++;
		
				token = strtok(NULL, " \n\r");
			}
		
			if (input_item_cnt != 1) {
				EMSG("Wrong number of input parameters. Retry!\n");
				goto RETRY;
			}
		
			if ((sel_idx >= 0) && (sel_idx < num_file))
				break;
		}
	}

	return sel_idx;
}

#if !defined(__KERNEL__) && defined(__linux__) /* Linux application */ || defined(__APPLE__)
static int make_download_file_list(const char *path,
								const char *file_ext_list[], int num_file_ext)
{
	DIR *dp;
	struct dirent *entry;
	struct stat st;
	char fullpath[256], *pos;
	int err, num_file = 0, i;
    size_t filename_len;
    
	if ((dp=opendir(path)) == NULL) {
		EMSG("Can't open directory: %s\n", path);
		return -1;
	}

	//DMSG("entry->d_name(%s)\n", entry->d_name);
	
	while((entry = readdir(dp)) != NULL) { // per file.
		sprintf(fullpath, "%s/%s", path, entry->d_name);
		//DMSG("fullpath(%s)\n", fullpath);

		err = lstat(fullpath, &st);
        if (err == -1) {
            EMSG("err = %d : %d : %s\n", err, errno, strerror(errno));
            num_file = -2;
            break;
        }
		else {
			if (S_ISREG(st.st_mode)) {
				//DMSG("FILE: %s\n", entry->d_name);

				filename_len = strlen(entry->d_name);				
				pos = entry->d_name + filename_len - 1;				
				while (*pos != '.')
					pos--;

				//DMSG("pos(%s)\n", pos);

				/* Find the specified file-extension then add name into list. */
				for (i = 0; i < num_file_ext; i++) {
					if (!strcasecmp(pos, file_ext_list[i])) {
						//strcpy(filename_list[num_file++], entry->d_name);
						sprintf(filename_list[num_file++],
							"%s/%s", path, entry->d_name);

						break;
					}				
				}
			}
		}
	}

	closedir(dp);

	return num_file;
}

#elif defined(_WIN32)
static int make_download_file_list(const char *path,
								const char *file_ext_list[], int num_file_ext)
{
	struct _finddata_t fd;
    long handle;
    int result = 1, i, num_file = 0;
	char filespec_buf[256];
 
	for (i = 0; i < num_file_ext; i++) {
		sprintf(filespec_buf, "%s\\*%s", path, file_ext_list[i]);

		handle = _findfirst(filespec_buf, &fd);
		if(handle == -1) {
			switch (errno)
			{
			case ENOENT: // Not found the specifed file extension.
				break;
			case EINVAL:
				EMSG("Invalid path name.\n");
				num_file = -1;
				break;
			case ENOMEM:
				EMSG("Not enough memory or file name too long.\n");
				num_file = -2;
				break;
			default:
				EMSG("Unknown error.\n");
				num_file = -3;
				break;
			}
			return num_file;
		}

		do {
			//DMSG("filename : %s\n", fd.name);
			sprintf(filename_list[num_file++], "%s\\%s", path, fd.name);
		} while((result = _findnext(handle, &fd)) == 0);

		_findclose(handle);
	}

	return num_file;
}
#endif

char *get_download_file_from_user(const char *path,
							const char *file_ext_list[], int num_file_ext)
{
	int num_file, sel_idx;

	DMSG("\tLUT file path: %s\n", path);

	num_file = make_download_file_list(path, file_ext_list, num_file_ext);
	if (num_file <= 0) {
		EMSG("No matching download files!\n");
		return NULL;
	}

	sel_idx = select_download_file(num_file);
	if (sel_idx >= 0)
		return  filename_list[sel_idx];
	else
		return NULL;
}

static void get_directory_name(char dir_name[512], const char * filename)
{
	size_t len;
	const char *last_path_separator;

#ifdef _WIN32
	last_path_separator = strrchr(filename, '\\');
	if (last_path_separator == NULL)
		last_path_separator = (const char *)strrchr(filename, '/');
#else
	last_path_separator = (const char *)strrchr(filename, '/');
#endif

	if (last_path_separator != NULL) {
		len = last_path_separator - filename;
		memcpy(dir_name, filename, len);
		dir_name[len] = '\0';
	}
	else {
		dir_name[0] = '.';
		dir_name[1] = '\0';
	}
}

static BOOL_T is_path_rooted(const char *path)
{
#ifdef _WIN32
	if ((isalpha(toupper(path[0])) != 0) && (path[1] == ':')) { // ex) C:
		return TRUE;
	} else
		return FALSE;
#else
	return (path[0] == '/');
#endif
}

static int get_item_full_path(char full_path[512], const char *item_path, const char *base_path)
{
	char *token, *path_separator_str;
	int path_separator_char;
	const char *last_path_separator, *tmp_str;
	const char *token_delimiters = "/\\";
	char temp_item_path[512];

	if (is_path_rooted(item_path) == TRUE) {
		strcpy(full_path, item_path);
		return 0;
	} else {
		/* A copy is used to protect the original input data. Due to strtok() */
		strcpy(temp_item_path, item_path);
		strcpy(full_path, base_path);

		/* Choose the path-separator to use when generating the full path. */
		tmp_str = (const char *)strchr(full_path, '\\');
		if (tmp_str != NULL) {
			path_separator_str = "\\";
			path_separator_char = '\\';
		}
		else {
			path_separator_str = "/";
			path_separator_char = '/';
		}

		token = strtok(temp_item_path, token_delimiters);
		while (token != NULL) {
			if (token[0] == '.') {
				 if (token[1] == '.') {
					last_path_separator = (const char *)strrchr(full_path, path_separator_char);
					full_path[last_path_separator - full_path] = '\0';
				 } else {
					 // Current directory
				 }
			} else {
				strcat(full_path, path_separator_str);
				strcat(full_path, token);
			}

			token = strtok(NULL, token_delimiters);
		}

		return 0;
	}
}

int test_Download_FileList(const char *filename, BOOL_T dload_activate,
							char *dump_path, UINT_T dump_flags)
{
	int i, ret = 0;
	static char read_buf[1024], idx_buf[32];
	FILE *fp_flist = NULL;
	int file_type_idx;
	int ok_flag, total_dload_list = 0;
	char *str_ptr;
	char base_path[512], full_path[512];

	fp_flist = fopen(filename, "rt");
	if (fp_flist == NULL) {
		EMSG("Can't open file: %s\n", filename);
		return -1;
	}

	vc_SetLutDumpInfomation(dump_path, dump_flags);

	ret = open_lut_dump_file();
	if (ret != 0) {
		fclose(fp_flist);
		return -2;
	}
	
	get_directory_name(base_path, filename);

	while (!feof(fp_flist)) {
READ_LINE:
		if (fgets(read_buf, 1024, fp_flist) == NULL)
			break;

		ok_flag = 0x0;

		str_ptr = trim_string(read_buf);
		if (str_ptr[0] == 0 || str_ptr[0] == '#') // null-character or Comment
			goto READ_LINE;

		// Index
		i = 0;
		while ((*str_ptr != '\0') && !isspace(*str_ptr))
			idx_buf[i++] = *str_ptr++;

		idx_buf[i] = '\0';
		if (is_string_digit(idx_buf) == TRUE) {
			file_type_idx = atoi(idx_buf);
			ok_flag = 0x1;
		}
		else {
			EMSG("NOT digit value!\n");
			ret = -3;
			goto error_exit;
		}

		// File path
		while ((*str_ptr != '\0') && isspace(*str_ptr)) // Find character position
			str_ptr++;

		if (*str_ptr == '\0') {
			EMSG("NOT found the file path string.\n");
			ret = -4;
			goto error_exit;
		}

		str_ptr = trim_right(str_ptr); /* Delete enter key */
		ret = get_item_full_path(full_path, str_ptr, base_path);
		ok_flag |= 0x2;

		if (ok_flag == 0x3) {
			total_dload_list++;

			DMSG("Prepare file: %s\n", full_path);

			switch (file_type_idx) {
			case 0: // Firmware hex
				ret = prepare_firmware_hex_file(full_path);
				break;

			case 1: // Firmware configuration
				ret = prepare_firmware_config_file(full_path);
				break;

			case 2: // RDC init LUT
				ret = prepare_rdc_init_lut(full_path);
				break;

			case 4: // Gamma LUT
				ret = prepare_panel_gamma_lut(full_path);
				break;

			case 3: // Panel init LUT
				ret = prepare_panel_init_lut(full_path);
				break;

			case 5: // CoKOA LUT
				ret = prepare_optic_dc_lut(full_path);
				break;

			case 8: // Font
				ret = prepare_font_lut(full_path);
				break;

			case 9: // Palette
				ret = prepare_palette_lut(full_path);
				break;

			case 10: // Bitmap
				ret = prepare_bmp_lut(full_path);
				break;

			default:
				EMSG("Invalid file type index\n");
				break;
			}

			DMSG("Prepare ret(%d)\n", ret);
			if (ret != 0) {
				fclose(fp_flist);
				goto error_exit;
			}
		}
	}

	fclose(fp_flist);

	close_lut_dump_file();

	if (dload_activate == TRUE) {
		if (total_dload_list != 0)
			rtiVC_StartDownload();
	}
	else {
		/* Free the allocated lut buffers. */
		rtiVC_RemoveAllSetupItem();
	}

	return 0;
	
error_exit:
	DMSG("Download Error [%d]\n", ret);
	rtiVC_RemoveAllSetupItem();

	close_lut_dump_file();

	return ret;
}


