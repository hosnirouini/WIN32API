

#include<windows.h>
#include<stdint.h>
#include<stdio.h>
#include<xinput.h>
#include<dsound.h>



//TODO :: sound buffer 

#define global   static
#define internal static
#define persist  static


typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32; 

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#if 1

#define WIDTH  900
#define HEIGHT 700

struct back_buffer
{
   BITMAPINFO bitmapinfo;
   void* bitmapmemory;
   int bitmapwidth;
   int bitmapheight;
   int pitch;  
};

struct window_dimension
{
  int width;
  int height;
};



global BOOL running ;
global back_buffer buffer;
global LPDIRECTSOUNDBUFFER secbuffer;


//get state
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex , XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(x_input_get_states)
{
  return (ERROR_DEVICE_NOT_CONNECTED);
}
global x_input_get_state *Xinput_get_state = x_input_get_states;
#define XInputGetState Xinput_get_state;

//set state
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex , XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(x_input_set_states)
{
  return (ERROR_DEVICE_NOT_CONNECTED);
}
global x_input_set_state *xinput_set_state = x_input_set_states;


#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND * ppDS, LPUNKNOWN  pUnkOuter )
typedef DIRECT_SOUND_CREATE(sound_create);




internal void
init_sound (HWND window , int32 size , uint32 SamplesPerSec )
{
  HMODULE sound_library = LoadLibraryA("dsound.dll");
  if(sound_library)
  {
    sound_create  *DirectSound = (sound_create*)GetProcAddress(sound_library,"DirectSoundCreate");
    LPDIRECTSOUND directSound;
    if(DirectSound && SUCCEEDED(DirectSound(0,&directSound,0)))
    {
       WAVEFORMATEX wave ={};
	  wave.wFormatTag = WAVE_FORMAT_PCM ;
	  wave.nChannels = 2;
	  wave.nSamplesPerSec  = SamplesPerSec;
	  wave.wBitsPerSample = 16;
	  wave.nBlockAlign =(wave.nChannels * wave.wBitsPerSample )/  8 ;
	  //SamplesPerSec;
	  wave.nAvgBytesPerSec = wave.nBlockAlign *  wave.nSamplesPerSec ;
	  wave.cbSize = 0;
	  
        if(SUCCEEDED(directSound->SetCooperativeLevel(window,DSSCL_PRIORITY)))
        {
  	  DSBUFFERDESC soundbuffer ={};
	  soundbuffer.dwSize = sizeof(soundbuffer);
	  soundbuffer.dwFlags  = DSBCAPS_PRIMARYBUFFER; 
	
        
	  LPDIRECTSOUNDBUFFER primbuffer;
	  if(SUCCEEDED(directSound->CreateSoundBuffer(&soundbuffer,&primbuffer,0)))
	  {
	    if( SUCCEEDED(primbuffer->SetFormat(&wave)))
	    {
	    }
	    else
	    {
	    }
	  }
	  else
	  {
 	  }
	  DSBUFFERDESC secsoundbuffer ={};
	  secsoundbuffer.dwSize = sizeof(soundbuffer);
	  secsoundbuffer.dwFlags  = 0;
	  secsoundbuffer.lpwfxFormat = &wave;
	  secsoundbuffer.dwBufferBytes = size;
        
	  if(SUCCEEDED(directSound->CreateSoundBuffer(&secsoundbuffer,&secbuffer,0)))
	  {
	  
	  }
        }
        else
        {
        }
      }
      else
      {    
      }
    }
    else
    {
    
    }
  
}

internal void
loadXinput()
{
  HMODULE XInputlibrary = LoadLibrary("xinput1_3.dll");
  if(XInputlibrary)
  {
    Xinput_get_state = (x_input_get_state*)GetProcAddress(XInputlibrary,"XInputGetState");
    if(!Xinput_get_state){Xinput_get_state = x_input_get_states;OutputDebugStringA("error in loading get func \n");}
    xinput_set_state = (x_input_set_state*)GetProcAddress(XInputlibrary,"XInputSetState");
    if(!xinput_set_state){xinput_set_state = x_input_set_states;}
  }
  else
  {
    OutputDebugStringA("error in loading xinput fucntion \n");
  }
}


internal window_dimension
get_window_dimension(HWND window)
{
  window_dimension result;
  RECT client;
  GetClientRect(window,&client);
  result.width = client.right - client.left;
  result.height = client.bottom - client.top ;

  return (result);
}


internal void
print_to_buffer(back_buffer *buffer,int r , int g , int b)
{
      buffer->pitch = buffer->bitmapwidth * 4;
      uint8 *Row = (uint8*)buffer->bitmapmemory;
      for (int x = 0;
	   x < buffer->bitmapheight;
	   ++x)
      {
	uint8 *pixel = (uint8*)Row ;
	for(int y = 0;
	    y < buffer->bitmapwidth;
	    ++y)
	{
	  
	  *pixel = r | (1<<8);
	    ++pixel;
	  
	    *pixel = g | (1<<8);
	    ++pixel ;

	    *pixel=  b | (1<<8);
	    ++pixel ;

	    *pixel = 0;
	    ++pixel ;
	  
	}
	Row += buffer->pitch ;
      }      
}


internal void
ResizeDibSection(back_buffer *buffer,
		 int width,int height)
{
  if(buffer->bitmapmemory)
  {
    VirtualFree(buffer->bitmapmemory,0,MEM_RELEASE);
  }

  buffer->bitmapwidth = width;
  buffer->bitmapheight = height;
  int bytesperpixel = 4;
  
  buffer->bitmapinfo.bmiHeader.biSize     = sizeof(buffer->bitmapinfo.bmiHeader);
  buffer->bitmapinfo.bmiHeader.biWidth    = buffer->bitmapwidth;
  buffer->bitmapinfo.bmiHeader.biHeight   = buffer->bitmapheight;
  buffer->bitmapinfo.bmiHeader.biPlanes   = 1;
  buffer->bitmapinfo.bmiHeader.biBitCount = 32;
  buffer->bitmapinfo.bmiHeader.biCompression = BI_RGB;
  int bitmapmemorysize = (buffer->bitmapwidth * buffer->bitmapheight) * bytesperpixel ; 
  buffer->bitmapmemory = VirtualAlloc(0,bitmapmemorysize, MEM_COMMIT , PAGE_READWRITE);
  buffer->pitch = width*bytesperpixel;

      
}


internal void
updatewindow(HDC context,
	     back_buffer *buffer,
	     int width , int height)
{
  
  StretchDIBits(context,
		0,0,buffer->bitmapwidth,buffer->bitmapheight,
		0,0,width,height,
		buffer->bitmapmemory,&buffer->bitmapinfo,
		DIB_RGB_COLORS, SRCCOPY);
}


LRESULT CALLBACK WndProc(HWND hWnd,
			 UINT message,
			 WPARAM wParam,
			 LPARAM lParam)  
{  

  PAINTSTRUCT P;
  HDC h;
  LRESULT result ; 
    switch (message)  
    {  
     case WM_ACTIVATEAPP:
     {
     }break;

     case WM_SIZE:
     {
       RECT window;
       GetClientRect(hWnd,&window);
       int width  = window.right - window.left;
       int height = window.bottom - window.top;
       ResizeDibSection(&buffer,width,height);
       
     }break;
     
     case WM_PAINT:
     {
       
       h = BeginPaint(hWnd,&P);
       window_dimension dimension = get_window_dimension(hWnd);
       updatewindow(h , &buffer ,dimension.width, dimension.height);
       EndPaint(hWnd,&P);
       
     }break;
     case WM_QUIT:
     {
       running  = false ;
       PostQuitMessage(0);
     }break;
     
     case WM_DESTROY:
     {
       running = false ;
        PostQuitMessage(0);
     }break;
     case WM_LBUTTONDOWN:
     case WM_SYSKEYDOWN:
     case WM_SYSKEYUP:
     case WM_KEYDOWN:
     case WM_KEYUP:
        {
            uint32 VKCode = wParam;
            //bool WasDown = ((lParam & (1 << 30)) != 0);
            //bool IsDown = ((lParam & (1 << 31)) == 0);
            //if(IsDown)
            //{
                if(VKCode == 'W')
                {
                }
                else if(VKCode == 'A')
                {
		  print_to_buffer(&buffer,255,255,255);
		  OutputDebugStringA("A \n");
                }
                else if(VKCode == 'S')
                {
		  print_to_buffer(&buffer,0,255,255);
                }
                else if(VKCode == 'D')
                {
		  print_to_buffer(&buffer,128,255,255);
                }
                else if(VKCode == 'Q')
                {
		  print_to_buffer(&buffer,255,255,0);
                }
                else if(VKCode == 'E')
                {
		  print_to_buffer(&buffer,255,0,255);
                }
                else if(VKCode == VK_UP)
                {
		  print_to_buffer(&buffer,90,70,60);
                }
                else if(VKCode == VK_LEFT)
                {
		  print_to_buffer(&buffer,255,10,50);
                }
                else if(VKCode == VK_DOWN)
                {
		  print_to_buffer(&buffer,0,128,50);
                }
                else if(VKCode == VK_RIGHT)
                {
		  print_to_buffer(&buffer,255,180,0);
                }
		else if (VKCode == MK_LBUTTON)
		{
		  print_to_buffer(&buffer,255,0,0);
		}
                else if(VKCode == VK_ESCAPE)
                {
                    OutputDebugStringA("ESCAPE: ");
                    /*if(IsDown)
                    {
                        OutputDebugStringA("IsDown ");
			}*/
                    /*if(WasDown)
                    {
                        OutputDebugStringA("WasDown");
			}*/
                    OutputDebugStringA("\n");
		    }
                else if(VKCode == VK_SPACE)
                {
                }
		bool32 altisdown = lParam & (1<<29);
		if ((VKCode == VK_F4) && altisdown)
		{
		  OutputDebugStringA("f4\n");
		  running = false;
		}
		//}
        } break;
    default:  
      result =  DefWindowProc(hWnd, message, wParam, lParam);  
      break;  
    }  
  
    return (result);  
}  

int CALLBACK
WinMain(HINSTANCE hInstance,HINSTANCE hPrevinstance,
	LPSTR lpCmdLine,int nCmdShow)
{
  loadXinput();
  WNDCLASS window = {};
  ResizeDibSection(&buffer,800,1200);
  window.style          = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  window.lpfnWndProc    = WndProc;
  window.hInstance      = hInstance;
  window.hbrBackground  = 0;
  window.lpszMenuName   = "game";
  window.lpszClassName  = "engine";
    

  if(!RegisterClass (&window))
  {
    MessageBox(0,"error ","fuckk u ",MB_OK);
    return (-1);
  }
  
    HWND hwindow = CreateWindowEx(0,window.lpszClassName,"game",
			          WS_OVERLAPPEDWINDOW|WS_VISIBLE,
		                  CW_USEDEFAULT,CW_USEDEFAULT,
		                  WIDTH,HEIGHT,0,0,
			          hInstance,0);
    
    if(!hwindow)
    {
      MessageBox(0,"mother fucker ","deammn",MB_OK);
      return (1);
    }

      
    running = true;
    MSG msg;
    int r = 0;
    int g = 0;
    int b = 0;
    int index = 0;
    HDC devicecontext = GetDC(hwindow);
    int samplespersec = 48000;
    int hz = 265;
    int wavecounter = 0;
    int waveperiod = samplespersec / hz;
    int bytespersample = sizeof(int16)*2 ;
    int buffersize = samplespersec * bytespersample ;
    init_sound(hwindow,samplespersec,samplespersec*bytespersample);
    while(running)
    {
      print_to_buffer(&buffer,r,g,b);     
      while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
      {
       if(msg.message > 0)
       {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
       }
      }
      
      for(DWORD ControllerIndex = 0;
          ControllerIndex < XUSER_MAX_COUNT;
          ++ControllerIndex)
      {
           XINPUT_STATE ControllerState;
           if(Xinput_get_state(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
           {
                        
               XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

               bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
               bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
               bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
               bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
               bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
               bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
               bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
               bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
               bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
               bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
               bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
               bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                                        
               int16 StickX = Pad->sThumbLX;
               int16 StickY = Pad->sThumbLY;
	       if (AButton)
	       {
	         r += 200;
	         g += 200;
	         b += 200;
	       }
	       else if (BButton)
	       {
		 print_to_buffer(&buffer,255,255,255);
	       }
	       else if (XButton)
	       {
		 print_to_buffer(&buffer,0,0,0);
	       }
	       else if (YButton)
	       {
		 
	         print_to_buffer(&buffer,255,10,0);
	       }
	       else if (Up)
	       {
		 print_to_buffer(&buffer,0,255,0);
	       }
	       
            }
	   
           else
           {
	     _XINPUT_VIBRATION vibration;
	     xinput_set_state(ControllerIndex,&vibration);              
           
           }
	   
        }
     DWORD playcursor;
     DWORD writecursor;
    if(SUCCEEDED(secbuffer->GetCurrentPosition(&playcursor , &writecursor)))
    {
	
      DWORD bytes = index * bytespersample % buffersize;
      DWORD bytestowrite ;
      if (bytes > playcursor)
      {
	bytestowrite = buffersize - bytes;
	bytestowrite += playcursor ;
	
      }
      else
      {
	bytestowrite = playcursor - bytes ; 
      }
      
      //DWORD writepointer = writecursor ;
      //DWORD bytestowrite = bytes;
     VOID *region1;
     DWORD sizereg1;
     VOID *region2;
     DWORD sizereg2;
     VOID* deammn ;
     if(SUCCEEDED(secbuffer->Lock(bytes ,bytestowrite ,
				  &region1 , &sizereg1 ,
				  &region2 , &sizereg2 ,
				  0 )))
     {
       int16 *sample1 = (int16 *) region1; 
       uint32 regionsample = sizereg1 / bytespersample ;
       uint32 regionsample2 = sizereg2 / bytespersample ;
       for(uint32 index = 0 ;
	  index < regionsample;
	  ++index)
      {
	if(wavecounter)
	{
	  wavecounter = waveperiod;
	}
	
 	int16 samplevalue = (index++/ (waveperiod/2) % 2) ? 8000 : -8000;
	*sample1++ = samplevalue;
	*sample1++ = samplevalue;
	
      }
          
      int16 *sample2 = (int16*)region2 ;
      for(uint32 index2 = 0;
	  index2 < regionsample2;
	  ++index2)
      {
        
	int16 samplevalue = (index++/ (waveperiod/2)% 2) ? 8000 : -8000;
	*sample2++ = samplevalue;
	*sample2++ = samplevalue;
	
      }
       
     }
    
         
      secbuffer->Play(0,0,DSBPLAY_LOOPING);
      secbuffer->Unlock(region1,sizereg1,region2,sizereg2);
     }
      window_dimension dimension = get_window_dimension(hwindow);
      updatewindow(devicecontext ,&buffer , dimension.width, dimension.height);
      r += 1;
      g += 2;
      b += 3;
      
      ReleaseDC(hwindow , devicecontext);
   }
    
  
  
  return (0);
}

#else



int main (int argc ,char ** argv)
{

  
  
  system("pause");
  return (0);
}
#endif
