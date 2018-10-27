/*
 * When a CAB file reaches this size, a new CAB will be created
 * automatically.  This is useful for fitting CAB files onto disks.
 *
 * If you want to create just one huge CAB file with everything in
 * it, change this to a very very large number.
 */
#define MEDIA_SIZE			300000000

/*
 * When a folder has this much compressed data inside it,
 * automatically flush the folder.
 *
 * Flushing the folder hurts compression a little bit, but
 * helps random access significantly.
 */
#define FOLDER_THRESHOLD	900000


/*
 * Compression type to use
 */

#define COMPRESSION_TYPE    tcompTYPE_MSZIP

/*
 * Memory allocation function
 */
FNFCIALLOC(mem_alloc)
{
	return malloc(cb);
}


/*
 * Memory free function
 */
FNFCIFREE(mem_free)
{
	free(memory);
}


/*
 * File i/o functions
 */
FNFCIOPEN(fci_open)
{
    int result;

    result = _open(pszFile, oflag, pmode);

    if (result == -1)
        *err = errno;

    return result;
}

FNFCIREAD(fci_read)
{
    unsigned int result;

    result = (unsigned int) _read(hf, memory, cb);

    if (result != cb)
        *err = errno;

    return result;
}

FNFCIWRITE(fci_write)
{
    unsigned int result;

    result = (unsigned int) _write(hf, memory, cb);

    if (result != cb)
        *err = errno;

    return result;
}

FNFCICLOSE(fci_close)
{
    int result;

    result = _close(hf);

    if (result != 0)
        *err = errno;

    return result;
}

FNFCISEEK(fci_seek)
{
    long result;

    result = _lseek(hf, dist, seektype);

    if (result == -1)
        *err = errno;

    return result;
}

FNFCIDELETE(fci_delete)
{
    int result;

    result = remove(pszFile);

    if (result != 0)
        *err = errno;

    return result;
}


/*
 * File placed function called when a file has been committed
 * to a cabinet
 */
FNFCIFILEPLACED(file_placed)
{
	CString strMsg;

	strMsg.Format(
		"placed file '%s' (size %d) added to cabinet '%s'\n",
		pszFile, 
		cbFile, 
		pccab->szCab
	);
	strMsg.TrimRight();
	CCompressFile::c_instance->m_Log->WriteLogEntry(strMsg,0);

	if (fContinuation)
	{
		strMsg.Format("      (Above file is a later segment of a continued file)\n");
		CCompressFile::c_instance->m_Log->WriteLogEntry(strMsg,0);
	}

	return 0;
}


/*
 * Function to obtain temporary files
 */
FNFCIGETTEMPFILE(get_temp_file)
{
    char    *psz;

    psz = _tempnam("","xx");            // Get a name
    if ((psz != NULL) && (strlen(psz) < (unsigned)cbTempName)) {
        strcpy(pszTempName,psz);        // Copy to caller's buffer
        free(psz);                      // Free temporary name buffer
        return TRUE;                    // Success
    }
    //** Failed
    if (psz) {
        free(psz);
    }

    return FALSE;
}


/*
 * Progress function
 */
FNFCISTATUS(progress)
{
	client_state	*cs;
	CString strErr;

	cs = (client_state *) pv;

	if (typeStatus == statusFile)
	{
        cs->total_compressed_size += cb1;
		cs->total_uncompressed_size += cb2;

		/*
		 * Compressing a block into a folder
		 *
		 * cb2 = uncompressed size of block
		 */
		strErr.Format("Compressing: %9ld -> %9ld             \r",
            cs->total_uncompressed_size,
            cs->total_compressed_size);
		
		fflush(stdout);
	}
	else if (typeStatus == statusFolder)
	{
		int	percentage;

		/*
		 * Adding a folder to a cabinet
		 *
		 * cb1 = amount of folder copied to cabinet so far
		 * cb2 = total size of folder
		 */
		percentage = CCompressFile::c_instance->get_percentage(cb1, cb2);

		strErr.Format("Copying folder to cabinet: %d%%      \r", percentage);

		fflush(stdout);
	}

	return 0;
}


FNFCIGETNEXTCABINET(get_next_cabinet)
{
	/*
	 * Cabinet counter has been incremented already by FCI
	 */

	/*
	 * Store next cabinet name
	 */
	CCompressFile::c_instance->store_cab_name(pccab->szCab, pccab->iCab);

	/*
	 * You could change the disk name here too, if you wanted
	 */

	return TRUE;
}


FNFCIGETOPENINFO(get_open_info)
{
	BY_HANDLE_FILE_INFORMATION	finfo;
	FILETIME					filetime;
	HANDLE						handle;
    DWORD                       attrs;
    int                         hf;

    /*
     * Need a Win32 type handle to get file date/time
     * using the Win32 APIs, even though the handle we
     * will be returning is of the type compatible with
     * _open
     */
	handle = CreateFile(
		pszName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);
   
	if (handle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	if (GetFileInformationByHandle(handle, &finfo) == FALSE)
	{
		CloseHandle(handle);
		return -1;
	}
   
	FileTimeToLocalFileTime(
		&finfo.ftLastWriteTime, 
		&filetime
	);

	FileTimeToDosDateTime(
		&filetime,
		pdate,
		ptime
	);

    attrs = GetFileAttributes(pszName);

    if (attrs == 0xFFFFFFFF)
    {
        /* failure */
        *pattribs = 0;
    }
    else
    {
        /*
         * Mask out all other bits except these four, since other
         * bits are used by the cabinet format to indicate a
         * special meaning.
         */
        *pattribs = (int) (attrs & (_A_RDONLY | _A_SYSTEM | _A_HIDDEN | _A_ARCH));
    }

    CloseHandle(handle);


    /*
     * Return handle using _open
     */
	hf = _open( pszName, _O_RDONLY | _O_BINARY );

	if (hf == -1)
		return -1; // abort on error
   
	return hf;
}
