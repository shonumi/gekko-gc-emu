#include "../emu.h"
#include "../dvd interface/realdvd.h"
#include "hle_func.h"
#include "hle_general.h"
#include <windows.h>
#include <string>
#include <cassert>
#include <vector>
#include <dsound.h>
#include "../cpu core/cpu_core_regs.h"

/* DSound stuff from MS */

LPDIRECTSOUND8 lpds;
LPDIRECTSOUNDBUFFER lpDsb;

#define DECODEBUFSIZE (2*32*1024)
#define PLAYBUFSIZE (DECODEBUFSIZE*3)

BOOL InitDirectSound(LPDIRECTSOUND8 *lplpds, HWND hwnd) {
	HRESULT hr;

	return FALSE;

//	hr = DirectSoundCreate8(NULL, lplpds, NULL);

	if (!SUCCEEDED(hr)) return FALSE;

//	hr = (*lplpds)->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);

	if (!SUCCEEDED(hr)) return FALSE;

	return TRUE;
}

BOOL CreateBasicBuffer( 
    LPDIRECTSOUND8 lpDirectSound, 
    LPDIRECTSOUNDBUFFER *lplpDsb,
	int srate) 
{ 
  PCMWAVEFORMAT pcmwf; 
  DSBUFFERDESC dsbdesc; 
  HRESULT hr; 
 
  return FALSE;

  // Set up wave format structure. 
  memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT)); 
  pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM; 
  pcmwf.wf.nChannels = 2; 
  pcmwf.wf.nSamplesPerSec = srate; 
  pcmwf.wf.nBlockAlign = 4; 
  pcmwf.wf.nAvgBytesPerSec = 
    pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign; 
  pcmwf.wBitsPerSample = 16; 
 
  // Set up DSBUFFERDESC structure. 
 
  memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); 
  dsbdesc.dwSize = sizeof(DSBUFFERDESC); 
  dsbdesc.dwFlags = 
    DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY; 
 
  dsbdesc.dwBufferBytes = PLAYBUFSIZE*2;
  dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf; 
 
  // Create buffer. 
 
//  hr = lpDirectSound->CreateSoundBuffer(&dsbdesc, lplpDsb, NULL); 
  if SUCCEEDED(hr) 
  { 
    // IDirectSoundBuffer interface is in *lplpDsb. 
    // Use QueryInterface to obtain IDirectSoundBuffer8.
    return TRUE; 
  } 
  else 
  { 
    // Failed. 
    *lplpDsb = NULL; 
    return FALSE; 
  } 
}

BOOL AppWriteDataToBuffer( 
    LPDIRECTSOUNDBUFFER lpDsb,  // The buffer.
    DWORD dwOffset,              // Our own write cursor.
    LPBYTE lpbSoundData,         // Start of our data.
    DWORD dwSoundBytes)          // Size of block to copy.
{ 
    LPVOID  lpvPtr1; 
    DWORD   dwBytes1; 
    LPVOID  lpvPtr2; 
    DWORD   dwBytes2; 
    HRESULT hr; 
 
	return FALSE;

    // Obtain memory address of write block. This will be in two parts
    // if the block wraps around.
 
//    hr = lpDsb->Lock(dwOffset, dwSoundBytes, &lpvPtr1, 
//        &dwBytes1, &lpvPtr2, &dwBytes2, 0); 
 
    // If the buffer was lost, restore and retry lock. 
 
    if (DSERR_BUFFERLOST == hr) 
 
    { 
        lpDsb->Restore(); 
        hr = lpDsb->Lock(dwOffset, dwSoundBytes, 
            &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0); 
    } 
    if SUCCEEDED(hr) 
    { 
        // Write to pointers. 
 
        CopyMemory(lpvPtr1, lpbSoundData, dwBytes1); 
        if (NULL != lpvPtr2) 
        { 
            CopyMemory(lpvPtr2, lpbSoundData+dwBytes1, dwBytes2); 
        } 
 
        // Release the data back to DirectSound. 
 
//        hr = lpDsb->Unlock(lpvPtr1, dwBytes1, lpvPtr2, 
//            dwBytes2); 
        if SUCCEEDED(hr) 
        { 
            // Success. 
            return TRUE; 
        } 
    } 
 
    // Lock, Unlock, or Restore failed. 
 
    return FALSE; 
}

/* */

unsigned short afccoef[16][2] =
{{0,0},
{0x0800,0},
{0,0x0800},
{0x0400,0x0400},
{0x1000,0xf800},
{0x0e00,0xfa00},
{0x0c00,0xfc00},
{0x1200,0xf600},
{0x1068,0xf738},
{0x12c0,0xf704},
{0x1400,0xf400},
{0x0800,0xf800},
{0x0400,0xfc00},
{0xfc00,0x0400},
{0xfc00,0},
{0xf800,0}};

BOOL audioIsDone;

DWORD WINAPI PlayAFCStream( LPVOID lpParam ) {
	short decodebuf[DECODEBUFSIZE];
	u8 buf[DECODEBUFSIZE/32*18];
	u32 dvdfilehandle = 0;
	int srate;
	int hist[2][2]; /* [channel][histno] */
	int curbuf=0;
	
	return 0;

	DWORD curplaycurs,curwritecurs=0;
	
	dvdfilehandle = (u32)lpParam;

	int filesize = RealDVDGetFileSize(dvdfilehandle);

	if(dvdfilehandle==NULL)
	{
		printf("StreamLibStart: Error opening.\n");
		return 0;
	}

	if (!InitDirectSound(&lpds,wnd.hWnd)) {
		printf("StreamLibStart: Error initing DirectSound\n");
		return 0;
	}
	
	RealDVDRead(dvdfilehandle,(u32 *)buf,0x20);
	filesize-=0x20;

	//for (int i=0;i<0x20;i++) printf("%02x ",buf[i]);
	//printf ("\n");

	srate = (buf[8] * 0x100l) | (buf[9]);
	printf("stream size %d\t%d Hz\n",filesize,srate);
	
	memset(hist,0,sizeof(hist));

	if (!CreateBasicBuffer(lpds, &lpDsb, srate)) {
		printf("StreamLibStart: Error creating buffer\n");
		return 0;
	}
	
	while (audioIsDone == false) { // While the thread is still alive
		DWORD  dwStatus;

		int amtread=RealDVDRead(dvdfilehandle,(u32*)buf,min(DECODEBUFSIZE/32*18,filesize));
		filesize-=amtread;
		for (int i=0;i<amtread/18*32;i+=32) {
			for (int chan=0;chan<2;chan++) {
				int delta,idx;
				
				delta = 1<<(((buf[(i/16+chan)*9])>>4)&0xf);
				idx = (buf[(i/16+chan)*9])&0xf;
				for (int j=0;j<16;j+=2) {
					int sample;
					int nybble[2];
					nybble[0]=buf[(i/16+chan)*9+1+j/2]>>4;
					nybble[1]=buf[(i/16+chan)*9+1+j/2]&0xf;

					for (int k=0;k<2;k++) {
						if (nybble[k]>=8) nybble[k]-=16;
					
						sample=hist[chan][0]*afccoef[idx][0]+hist[chan][1]*afccoef[idx][1]+((nybble[k]*delta)<<11);
						sample>>=11;
						if(sample > 32767) {
							sample = 32767;
						}
						if(sample < -32768) {
							sample = -32768;
						}
						hist[chan][1]=hist[chan][0];
						hist[chan][0]=sample;
						decodebuf[i+(j+k)*2+chan]=sample;
					}
				}
			}
		}

//		lpDsb->GetStatus(&dwStatus);

		int space;

		if ((dwStatus & DSBSTATUS_PLAYING)) {
			do {
				Sleep(100);
				lpDsb->GetCurrentPosition( &curplaycurs, NULL);
				space = curplaycurs-curwritecurs;
				if (space<0) space+=PLAYBUFSIZE*2;
			} while (space<sizeof(decodebuf));
		} else {
			curwritecurs=0;
		}

		//printf("new buffer\n");
		//for (int i=0;i<0x20;i++) printf("%02x ",buf[i]);

		AppWriteDataToBuffer( 
			lpDsb,
			curwritecurs,
			(u8*)decodebuf,
			sizeof(decodebuf)
			);

		curwritecurs+=sizeof(decodebuf);
		if (curwritecurs>=PLAYBUFSIZE*2) curwritecurs=0;

//		lpDsb->GetStatus(&dwStatus);
		if (!(dwStatus & DSBSTATUS_PLAYING)) {
/*			HRESULT hr = lpDsb->Play(
				0,    // Unused.
				0,    // Priority for voice management.
				DSBPLAY_LOOPING);   // Flags.
*/		}

		if (filesize<18) {
			printf("End of stream\n");
			break;
		}
	}

#pragma todo("we need a way to let the rest of the buffer play out")
#pragma todo("TODO: looping")
//	lpDsb->Release();
//	lpds->Release();

	return 0;
}

HANDLE stream;

u32 streamfilehandle = 0;

HLE(StreamLibStart)
{
	return;
/*
	char * filepath = (char*)(&RAM[GPR(3)&RAM_MASK]);

	printf("Play stream: %s\n",filepath);
	
	audioIsDone=false;

	streamfilehandle = RealDVDOpen(filepath);

	stream = CreateThread( NULL, 0, 
           PlayAFCStream, (void*)streamfilehandle, 0, NULL);

	SetThreadPriority(stream,THREAD_PRIORITY_HIGHEST);
*/
}

HLE(StreamLibStop)
{
	return;

	printf("Stop stream.\n");

	RealDVDClose(streamfilehandle);
	streamfilehandle=0;

	audioIsDone=true;
}