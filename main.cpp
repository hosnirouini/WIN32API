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

#define WIDTH  1280
#define HEIGHT 800
int r = 0;
int g = 0;
int b = 0;


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
init_sound (HWND window , int32 size , int32 SamplesPerSec )
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
	  wave.nBlockAlign =(wave.nChannels * wave.wBitsPerSample )/8 ;
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
	      OutputDebugStringA("primary buffer format was set \n");
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
	  DSBUFFERDESC secsoundbuffer = {};
	  secsoundbuffer.dwSize = sizeof(secsoundbuffer);
	  secsoundbuffer.dwFlags  = 0;
	  secsoundbuffer.dwBufferBytes = size;
	  secsoundbuffer.lpwfxFormat = &wave;
	  
        
	if(SUCCEEDED(directSound->CreateSoundBuffer(&secsoundbuffer,&secbuffer,0)))
	{
	  OutputDebugStringA("secondary buffer created successfully \n");
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
Render_buffer(back_buffer *buffer,int r , int g , int b)
{
  //buffer->pitch = buffer->bitmapwidth * 4;
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
	  
	  *pixel = (uint8)(x+r);
	    ++pixel;
	  
	    *pixel = (uint8)(y+g);
	    ++pixel ;

	    *pixel= 0 ;
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

  buffer->bitmapwidth  = width;
  buffer->bitmapheight = height;
  int bytesperpixel = 4;
  
  buffer->bitmapinfo.bmiHeader.biSize     = sizeof(buffer->bitmapinfo.bmiHeader);
  buffer->bitmapinfo.bmiHeader.biWidth    = buffer->bitmapwidth;
  buffer->bitmapinfo.bmiHeader.biHeight   = buffer->bitmapheight;
  buffer->bitmapinfo.bmiHeader.biPlanes   = 1;
  buffer->bitmapinfo.bmiHeader.biBitCount = 32;
  buffer->bitmapinfo.bmiHeader.biCompression = BI_RGB;
  int bitmapmemorysize = (buffer->bitmapwidth * buffer->bitmapheight) * bytesperpixel ; 
  buffer->bitmapmemory = VirtualAlloc(0,bitmapmemorysize, MEM_RESERVE|MEM_COMMIT , PAGE_READWRITE);
  buffer->pitch = width * bytesperpixel;
      
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
       HDC DeviceContext = BeginPaint(hWnd , &P);
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
		  //print_to_buffer(&buffer,255,255,255);
		  r+= 10;
		  g+= 10;
		  OutputDebugStringA("A \n");
                }
                else if(VKCode == 'S')
                {
		  //print_to_buffer(&buffer,0,255,255);
                }
                else if(VKCode == 'D')
                {
		  
		  r+= 10;
		  g+= 10;
		  //print_to_buffer(&buffer,128,255,255);
                }
                else if(VKCode == 'Q')
                {
		  r+= 10;
		  g+= 10;
		  //Render_buffer(&buffer,255,255,0);
                }
                else if(VKCode == 'E')
                {
		  Render_buffer(&buffer,255,0,255);
                }
                else if(VKCode == VK_UP)
                {
		  //print_to_buffer(&buffer,90,70,60);
                }
                else if(VKCode == VK_LEFT)
                {
		  //print_to_buffer(&buffer,255,10,50);
                }
                else if(VKCode == VK_DOWN)
                {
		  //print_to_buffer(&buffer,0,128,50);
                }
                else if(VKCode == VK_RIGHT)
                {
		  //print_to_buffer(&buffer,255,180,0);
                }
		else if (VKCode == MK_LBUTTON)
		{
		  //print_to_buffer(&buffer,255,0,0);
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
  WNDCLASSEXA window = {};
  ResizeDibSection(&buffer,800,1200);
  window.cbSize = sizeof(WNDCLASSEXA);
  window.style          = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  window.lpfnWndProc    = WndProc;
  window.hInstance      = hInstance;
  window.lpszClassName  = "engine";
    


  if(RegisterClassExA (&window))
    {
  
      HWND hwindow = CreateWindowEx(0,window.lpszClassName,"game",
			          WS_OVERLAPPEDWINDOW|WS_VISIBLE,
		                  CW_USEDEFAULT,CW_USEDEFAULT,
		                  WIDTH,HEIGHT,0,0,
			          hInstance,0);
  
   
    if (hwindow)
    {
   
      HDC devicecontext = GetDC(hwindow);
      	int SamplesPerSecond = 48000;
	int ToneHz = 256;
	int16 ToneVolume = 6000;
	uint32 RunningSampleIndex = 0;
	int SquareWavePeriod = SamplesPerSecond / ToneHz;
	int HalfSquareWavePeriod = SquareWavePeriod / 2;
	int BytesPerSample = sizeof(int16) * 2;
	int SecondaryBufferSize = SamplesPerSecond * BytesPerSample;

	init_sound(hwindow, SecondaryBufferSize, SamplesPerSecond);
        secbuffer->Play(0, 0, DSBPLAY_LOOPING);

     running = true;
     while(running)
     {
        MSG msg;       
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
	       }
	       else if (BButton)
	       {
	       }
	       else if (XButton)
	       {
	       }
	       else if (YButton)
	       {
	       }
	       else if (Up)
	       {
	       }
           }
           else
           {
	     _XINPUT_VIBRATION vibration;
	     xinput_set_state(ControllerIndex,&vibration);              
           
           }
	   
     }
   DWORD PlayCursor;
				DWORD WriteCursor;

				if (SUCCEEDED(secbuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
				{
					DWORD ByteToLock = (RunningSampleIndex * BytesPerSample) % SecondaryBufferSize;
					DWORD BytesToWrite;

					if (ByteToLock == PlayCursor)
					{
						BytesToWrite = SecondaryBufferSize;
					}
					else if (ByteToLock > PlayCursor)
					{
						BytesToWrite = SecondaryBufferSize - ByteToLock;
						BytesToWrite += PlayCursor;
					}
					else
					{
						BytesToWrite = PlayCursor - ByteToLock;
					}

					// TODO(matthewr): More testing!
					// TODO(matthewr): Switch to a sine wave
					VOID *Region1;
					DWORD Region1Size;
					VOID *Region2;
					DWORD Region2Size;

					if (SUCCEEDED(secbuffer->Lock(ByteToLock, BytesToWrite,
															  &Region1, &Region1Size,
															  &Region2, &Region2Size, 0)))
					{
						// TODO(matthewr): Assert that RegionXSize is valid
						// TODO(matthewr): Collapse these two loops
						DWORD Region1SampleCount = Region1Size / BytesPerSample;
						int16 *SampleOut = (int16 *)Region1;
						
						for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
						{
							int16 SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2) ? ToneVolume : -ToneVolume;

							*SampleOut++ = SampleValue;
							*SampleOut++ = SampleValue;
						}

						DWORD Region2SampleCount = Region2Size / BytesPerSample;
						SampleOut = (int16 *)Region2;
						for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
						{
		     	int16 SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2) ? ToneVolume : -ToneVolume;

							*SampleOut++ = SampleValue;
							*SampleOut++ = SampleValue;
						}

						secbuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
					}
				}

   Render_buffer(&buffer,r,g,b);
   r+= 5;
   g+=5;		   
   window_dimension dimension = get_window_dimension(hwindow);
   updatewindow(devicecontext ,&buffer , dimension.width, dimension.height);
  }


  
  }
  else
  {
    OutputDebugStringA("error in sound mother fucker \n");
  }
 }
 else
 {
   OutputDebugStringA("error in sound  \n");
 }
  return (0);
}
#endif
