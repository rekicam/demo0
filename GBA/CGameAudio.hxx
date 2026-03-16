// Copyripht 2019 moecmks (agalis01@outlook.com)
// This file is part of ArchBoyAdvance.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 

#if _MSC_VER >= 1000
# pragma once
#endif // _MSC_VER >= 1000

#if !defined (_CAGB_AUDIO_INCLUED_HEADER_CXX_)
# define _CAGB_AUDIO_INCLUED_HEADER_CXX_

# include <dsound.h>
# include "dx90csdk\Include\XAudio2.h"
# include <Windows.h>
# include <mmsystem.h>
# include <atldef.h>
# include "stdwtl.h"




struct CGameAudioSample16_44100Hz :  public IXAudio2VoiceCallback {
  static const int buffer_chunk_nums = 16;
  static const int buffer_frame_size = 4410;

  IXAudio2 *m_comIXAudio;
  IXAudio2MasteringVoice *m_comIXAudioMaster;
  IXAudio2SourceVoice *m_comIXAudioMixer;
  WAVEFORMATEX m_cWaveformat;

  void __stdcall OnStreamEnd() {}
  void __stdcall OnVoiceProcessingPassEnd() {  }
  void __stdcall OnVoiceProcessingPassStart (UINT32 SamplesRequired) {}
  void __stdcall OnBufferEnd (void * pBufferContext) { SetEvent (m_hBufferEndEvent); }
  void __stdcall OnBufferStart (void * pBufferContext) {}
  void __stdcall OnLoopEnd (void * pBufferContext) {}
  void __stdcall OnVoiceError (void * pBufferContext, HRESULT Error) { }

  HANDLE m_hRenderThread;
  HANDLE m_hBufferEndEvent;
  HANDLE m_hRenderQuitEvent;
  PBYTE m_bBufferBlock[buffer_chunk_nums];
  INT m_iBufferNext;
  INT m_iFrameSize;

  CGameAudioSample16_44100Hz (void) : 
                       m_iBufferNext (0), 
                       m_hRenderThread (NULL), 
                           m_hBufferEndEvent (NULL), 
                             m_hRenderQuitEvent (NULL) {

    m_iFrameSize = 44100 * 4 / 59 & -4;

    // malloc buffer.
    for (int id= 0; id != buffer_chunk_nums; id++) {
      m_bBufferBlock[id] = (PBYTE) _aligned_malloc (m_iFrameSize, 64);
      ZeroMemory (m_bBufferBlock[id], m_iFrameSize);
      ATLASSERT (m_bBufferBlock[id] != NULL);
    }
    HRESULT sig = XAudio2Create (& m_comIXAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    ATLASSERT (SUCCEEDED (sig));

    // re-build IXAudio2MasteringVoice/IXAudio2SourceVoiceobject 
    //  m_XAudio2Out->CreateSourceVoice
    sig = m_comIXAudio->CreateMasteringVoice (& m_comIXAudioMaster);
    ATLASSERT (SUCCEEDED (sig));

    // Reset format. 
    m_cWaveformat.wFormatTag = WAVE_FORMAT_PCM;
    m_cWaveformat.nChannels = 2; // always 2 
    m_cWaveformat.nSamplesPerSec = 44100;
    m_cWaveformat.wBitsPerSample = 16;
    m_cWaveformat.nBlockAlign = m_cWaveformat.wBitsPerSample/8 *m_cWaveformat.nChannels;
    m_cWaveformat.nAvgBytesPerSec = m_cWaveformat.nBlockAlign * m_cWaveformat.nSamplesPerSec;
    m_cWaveformat.cbSize = 0;

    // sig = sm_XAudio2->CreateSourceVoice (& m_XAudio2Mixer, & m_waveformat, XAUDIO2_VOICE_NOSRC); // Dis Sample rate cov.(SRC PASS)
    sig = m_comIXAudio->CreateSourceVoice (& m_comIXAudioMixer, & m_cWaveformat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this);
    ATLASSERT (SUCCEEDED (sig));

    sig = m_comIXAudioMixer->Stop();
    ATLASSERT (SUCCEEDED (sig));

    sig = m_comIXAudioMixer->FlushSourceBuffers();
    ATLASSERT (SUCCEEDED (sig));

    sig = m_comIXAudioMixer->Start();
    ATLASSERT (SUCCEEDED (sig));

    m_hBufferEndEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    m_hRenderQuitEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
  }

  void StartupRenderThread (void) {
    if (m_hRenderThread == NULL) {
      m_hRenderThread = (HANDLE) _beginthreadex (NULL, 0, render_thread, this, 0, 0);
      Sleep (15);
      ATLASSERT (m_hRenderThread != NULL);

      SetEvent (m_hBufferEndEvent);
    }
  }
  static unsigned int __stdcall render_thread (void *param) {
# if 0
    CGameAudioSample16_44100Hz *pThis = (CGameAudioSample16_44100Hz *)param;

    HANDLE hEventGroup[2];
    hEventGroup[0] = pThis->m_hRenderQuitEvent;
    hEventGroup[1] = pThis->m_hBufferEndEvent;

    do {

      XAUDIO2_VOICE_STATE state;

      switch (WaitForMultipleObjects (2, hEventGroup, FALSE, INFINITE)) {
      case WAIT_OBJECT_0 + 0:
        // Render thread end command.
        pThis->m_comIXAudioMixer->FlushSourceBuffers ();
        pThis->m_comIXAudioMixer->Stop ();
        return 0;
      case WAIT_OBJECT_0 + 1:
        pThis->m_comIXAudioMixer->GetState (& state);
        if (state.BuffersQueued <= buffer_chunk_nums/ 2) {
          // submit buffer.
          XAUDIO2_BUFFER buffer;
          ZeroMemory (& buffer, sizeof (XAUDIO2_BUFFER));
          buffer.pAudioData = (const BYTE *)pThis->m_bBufferBlock[pThis->m_iBufferNext++];
          buffer.AudioBytes = pThis->m_iFrameSize;
          pThis->m_iBufferNext %= buffer_chunk_nums; 
          pThis->m_comIXAudioMixer->SubmitSourceBuffer (& buffer);
        }
        break;
      default:
        ATLASSERT (FALSE);
        break;
      }
    } while (TRUE);
#else 
    CGameAudioSample16_44100Hz *pThis = (CGameAudioSample16_44100Hz *)param;
    pThis->m_comIXAudioMixer->Start ();
    do {
        XAUDIO2_VOICE_STATE state;
        pThis->m_comIXAudioMixer->GetState (& state);
        if (state.BuffersQueued <= buffer_chunk_nums/ 2) {
          // submit buffer.
          XAUDIO2_BUFFER buffer;
          ZeroMemory (& buffer, sizeof (XAUDIO2_BUFFER));
          buffer.pAudioData = (const BYTE *)pThis->m_bBufferBlock[pThis->m_iBufferNext++];
          buffer.AudioBytes = pThis->m_iFrameSize;
          pThis->m_iBufferNext %= buffer_chunk_nums; 
          pThis->m_comIXAudioMixer->SubmitSourceBuffer (& buffer);

          // pThis->m_comIXAudioMixer->Stop ();
        } else {
          // Sleep (5);
          // pThis->m_comIXAudioMixer->Start ();
        }
    } while (TRUE);

#endif 
  }

  void GetBuffer (void **pData) {
    int iBankPos = (m_iBufferNext - 1) % buffer_chunk_nums;
    if (iBankPos < 0)
      iBankPos = buffer_chunk_nums + iBankPos;
    * pData = m_bBufferBlock[ iBankPos];
  }
  void *GetBuffer (void) {
    int iBankPos = (m_iBufferNext - 1) % buffer_chunk_nums;
    if (iBankPos < 0)
      iBankPos = buffer_chunk_nums + iBankPos;
    return m_bBufferBlock[ iBankPos];
  }
  int GetFrameSize (void) {
    return m_iFrameSize;
  }
};

struct CGameAudioSample16_44100Hz_2 {

  static const DWORD BufferTotal = 1470 * 2 * 10;
  static const DWORD BufferUnit = 1470 * 2;

  LPDIRECTSOUND        m_DirectSound;
  LPDIRECTSOUNDBUFFER  m_PrimaryBuffer;
  LPDIRECTSOUNDBUFFER  m_SecondaryBuffer;
  WAVEFORMATEX         m_Waveformat;

  DWORD m_soundNextPosition;
  HWND m_AttachWindow;

  CGameAudioSample16_44100Hz_2 (HWND attachWindow)
    : m_AttachWindow (attachWindow),
      m_soundNextPosition (0) {}

  void reset (void);

  void write (void *frame) {
  
    HRESULT result;
    DWORD status=0;
    LPVOID  lpvPtr1; 
    DWORD   dwBytes1; 
    LPVOID  lpvPtr2; 
    DWORD   dwBytes2; 


    if(1) {  
      DWORD play;
      while(true) {
        m_SecondaryBuffer->GetCurrentPosition(& play, NULL);

        if(play < m_soundNextPosition ||
            play > m_soundNextPosition+BufferUnit) {
          break;
        }
      }
    } 


    // Obtain memory address of write block. This will be in two parts
    // if the block wraps around.
    result = m_SecondaryBuffer->Lock (m_soundNextPosition, BufferUnit, & lpvPtr1, 
                            & dwBytes1, & lpvPtr2, & dwBytes2, 0);
  
    // If DSERR_BUFFERLOST is returned, restore and retry lock. 
    if (DSERR_BUFFERLOST == result) { 
      m_SecondaryBuffer->Restore(); 
      result = m_SecondaryBuffer->Lock (m_soundNextPosition, BufferUnit,&lpvPtr1,
                              &dwBytes1, &lpvPtr2, &dwBytes2,
                              0);
    } 

    m_soundNextPosition += BufferUnit;
    m_soundNextPosition = m_soundNextPosition % BufferTotal;
  
    if SUCCEEDED(result) { 
      // Write to pointers. 
      CopyMemory(lpvPtr1, frame, dwBytes1); 
      if (NULL != lpvPtr2) { 
        CopyMemory(lpvPtr2, (uint8_t *) frame+dwBytes1, dwBytes2); 
      } 
      // Release the data back to DirectSound. 
      result = m_SecondaryBuffer->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
    }
  }

  void init (void) {

    HRESULT result = DirectSoundCreate (NULL, & m_DirectSound, NULL);
    ATLASSERT (SUCCEEDED (result));

    result = m_DirectSound->SetCooperativeLevel (m_AttachWindow, DSSCL_PRIORITY | DSSCL_NORMAL);
    ATLASSERT (SUCCEEDED (result));

    DSBUFFERDESC dsbdesc;
    ZeroMemory (& dsbdesc, sizeof(DSBUFFERDESC));
    dsbdesc.dwSize = sizeof (DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

    result = m_DirectSound->CreateSoundBuffer (& dsbdesc, & m_PrimaryBuffer, NULL);
    ATLASSERT (SUCCEEDED (result));

    memset (& m_Waveformat, 0, sizeof (WAVEFORMATEX)); 
    m_Waveformat.wFormatTag = WAVE_FORMAT_PCM; 
    m_Waveformat.nChannels = 2;
    m_Waveformat.nSamplesPerSec = 44100;
    m_Waveformat.wBitsPerSample = 16; 
    m_Waveformat.nBlockAlign = (m_Waveformat.wBitsPerSample / 8) * m_Waveformat.nChannels;
    m_Waveformat.nAvgBytesPerSec = m_Waveformat.nSamplesPerSec * m_Waveformat.nBlockAlign;

    result = m_PrimaryBuffer->SetFormat (& m_Waveformat);
    ATLASSERT (SUCCEEDED (result));

    ZeroMemory (& dsbdesc, sizeof (DSBUFFERDESC));  
    dsbdesc.dwSize = sizeof (DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
    dsbdesc.dwBufferBytes = BufferTotal;
    dsbdesc.lpwfxFormat = & m_Waveformat;

    result = m_DirectSound->CreateSoundBuffer (& dsbdesc, & m_SecondaryBuffer, NULL);
    ATLASSERT (SUCCEEDED (result));

    m_SecondaryBuffer->SetCurrentPosition (0);
    m_SecondaryBuffer->Play (0, 0, DSBPLAY_LOOPING);
  }



  void pause (void);
  void resume (void);
  void play (void);

};


#endif