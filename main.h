//d3d11 aimbot for warframe by n7

//==========================================================================================================================

//features deafult settings
int Folder1 = 1;
int Item1 = 1; //sOptions[0].Function //esp
int Item2 = 1; //sOptions[1].Function //aimbot pve
int Item3 = 0; //sOptions[2].Function //aimbot pvp
int Item4 = 0; //sOptions[3].Function //aimkey 0 = RMouse
int Item5 = 3; //sOptions[4].Function //aimsens
int Item6 = 4; //sOptions[5].Function //aimfov
int Item7 = 4; //sOptions[6].Function //aimheight
int Item8 = 0; //sOptions[7].Function //autoshoot

//globals
DWORD Daimkey = VK_RBUTTON;		//aimkey
int aimheight = 46;				//aim height value
unsigned int asdelay = 90;		//use x-999 (shoot for xx millisecs, looks more legit)
bool IsPressed = false;			//
DWORD astime = timeGetTime();	//autoshoot timer

//init only once
bool firstTime = true;

//viewport
UINT vps = 1;
D3D11_VIEWPORT viewport;
float ScreenCenterX;
float ScreenCenterY;

//vertex
ID3D11Buffer *veBuffer;
UINT Stride = 0;
UINT veBufferOffset = 0;
D3D11_BUFFER_DESC vedesc;

//index
ID3D11Buffer *inBuffer;
DXGI_FORMAT inFormat;
UINT        inOffset;
D3D11_BUFFER_DESC indesc;

//rendertarget
ID3D11Texture2D* RenderTargetTexture;
ID3D11RenderTargetView* RenderTargetView = NULL;

//shader
ID3D11PixelShader* psRed = NULL;
ID3D11PixelShader* psGreen = NULL;

//pssetshaderresources
UINT pssrStartSlot;
D3D11_SHADER_RESOURCE_VIEW_DESC  Descr;
ID3D11ShaderResourceView* ShaderResourceView;
D3D11_TEXTURE2D_DESC texdesc;

//psgetConstantbuffers
ID3D11Buffer *pcsBuffer;
D3D11_BUFFER_DESC pscdesc;
UINT pscStartSlot;

#define SAFE_RELEASE(x) if (x) { x->Release(); x = NULL; }

//==========================================================================================================================

//get dir
using namespace std;
#include <fstream>
char dlldir[320];
char *GetDirectoryFile(char *filename)
{
	static char path[320];
	strcpy_s(path, dlldir);
	strcat_s(path, filename);
	return path;
}

//==========================================================================================================================

//menu
#define MAX_ITEMS 25

#define T_FOLDER 1
#define T_OPTION 2
#define T_MULTIOPTION 3
#define T_AIMKEYOPTION 4

#define LineH 15

struct Options {
	LPCWSTR Name;
	int	Function;
	BYTE Type;
};

struct Menu {
	LPCWSTR Title;
	int x;
	int y;
	int w;
};

DWORD Color_Font;
DWORD Color_On;
DWORD Color_Off;
DWORD Color_Folder;
DWORD Color_Current;

bool Is_Ready, Visible;
int Items, Cur_Pos;

Options sOptions[MAX_ITEMS];
Menu sMenu;

#include <string>
#include <fstream>
void SaveCfg()
{
	ofstream fout;
	fout.open(GetDirectoryFile("wfd3d11.ini"), ios::trunc);
	fout << "Item1 " << sOptions[0].Function << endl;
	fout << "Item2 " << sOptions[1].Function << endl;
	fout << "Item3 " << sOptions[2].Function << endl;
	fout << "Item4 " << sOptions[3].Function << endl;
	fout << "Item5 " << sOptions[4].Function << endl;
	fout << "Item6 " << sOptions[5].Function << endl;
	fout << "Item7 " << sOptions[6].Function << endl;
	fout << "Item8 " << sOptions[7].Function << endl;
	fout.close();
}

void LoadCfg()
{
	ifstream fin;
	string Word = "";
	fin.open(GetDirectoryFile("wfd3d11.ini"), ifstream::in);
	fin >> Word >> Item1;
	fin >> Word >> Item2;
	fin >> Word >> Item3;
	fin >> Word >> Item4;
	fin >> Word >> Item5;
	fin >> Word >> Item6;
	fin >> Word >> Item7;
	fin >> Word >> Item8;
	fin.close();
}

void JBMenu(void)
{
	Visible = true;
}

void Init_Menu(ID3D11DeviceContext *pContext, LPCWSTR Title, int x, int y)
{
	Is_Ready = true;
	sMenu.Title = Title;
	sMenu.x = x;
	sMenu.y = y;
}

void AddFolder(LPCWSTR Name, int Pointer)
{
	sOptions[Items].Name = (LPCWSTR)Name;
	sOptions[Items].Function = Pointer;
	sOptions[Items].Type = T_FOLDER;
	Items++;
}

void AddOption(LPCWSTR Name, int Pointer, int *Folder)
{
	if (*Folder == 0)
		return;
	sOptions[Items].Name = Name;
	sOptions[Items].Function = Pointer;
	sOptions[Items].Type = T_OPTION;
	Items++;
}

void AddMultiOption(LPCWSTR Name, int Pointer, int *Folder)
{
	if (*Folder == 0)
		return;
	sOptions[Items].Name = Name;
	sOptions[Items].Function = Pointer;
	sOptions[Items].Type = T_MULTIOPTION;
	Items++;
}

void AddMultiOptionText(LPCWSTR Name, int Pointer, int *Folder)
{
	if (*Folder == 0)
		return;
	sOptions[Items].Name = Name;
	sOptions[Items].Function = Pointer;
	sOptions[Items].Type = T_AIMKEYOPTION;
	Items++;
}

void Navigation()
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		SaveCfg(); //save settings
		Visible = !Visible;
	}

	if (!Visible)
		return;

	int value = 0;

	if (GetAsyncKeyState(VK_DOWN) & 1)
	{
		Cur_Pos++;
		if (sOptions[Cur_Pos].Name == 0)
			Cur_Pos--;
	}

	if (GetAsyncKeyState(VK_UP) & 1)
	{
		Cur_Pos--;
		if (Cur_Pos == -1)
			Cur_Pos++;
	}

	else if (sOptions[Cur_Pos].Type == T_OPTION && GetAsyncKeyState(VK_RIGHT) & 1)
	{
		if (sOptions[Cur_Pos].Function == 0)
			value++;
	}

	else if (sOptions[Cur_Pos].Type == T_OPTION && (GetAsyncKeyState(VK_LEFT) & 1) && sOptions[Cur_Pos].Function == 1)
	{
		value--;
	}

	else if (sOptions[Cur_Pos].Type == T_MULTIOPTION && GetAsyncKeyState(VK_RIGHT) & 1 && sOptions[Cur_Pos].Function <= 6)//max
	{
		value++;
	}

	else if (sOptions[Cur_Pos].Type == T_MULTIOPTION && (GetAsyncKeyState(VK_LEFT) & 1) && sOptions[Cur_Pos].Function != 0)
	{
		value--;
	}

	else if (sOptions[Cur_Pos].Type == T_AIMKEYOPTION && GetAsyncKeyState(VK_RIGHT) & 1 && sOptions[Cur_Pos].Function <= 6)//max
	{
		value++;
	}

	else if (sOptions[Cur_Pos].Type == T_AIMKEYOPTION && (GetAsyncKeyState(VK_LEFT) & 1) && sOptions[Cur_Pos].Function != 0)
	{
		value--;
	}


	if (value) {
		sOptions[Cur_Pos].Function += value;
		if (sOptions[Cur_Pos].Type == T_FOLDER)
		{
			memset(&sOptions, 0, sizeof(sOptions));
			Items = 0;
		}
	}

}

bool IsReady()
{
	if (Items)
		return true;
	return false;
}

void DrawTextF(ID3D11DeviceContext* pContext, LPCWSTR text, int FontSize, int x, int y, DWORD Col)
{
	if (Is_Ready == false)
		MessageBoxA(0, "Error, you dont initialize the menu!", "Error", MB_OK);

	if (pFontWrapper)
		pFontWrapper->DrawString(pContext, text, (float)FontSize, (float)x, (float)y, Col, FW1_RESTORESTATE);
}

void Draw_Menu()
{
	if (!Visible)
		return;

	DrawTextF(pContext, sMenu.Title, 14, sMenu.x + 10, sMenu.y, Color_Font);
	for (int i = 0; i < Items; i++)
	{
		if (sOptions[i].Type == T_OPTION)
		{
			if (sOptions[i].Function)
			{
				DrawTextF(pContext, L"On", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);
			}
			else {
				DrawTextF(pContext, L"Off", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_Off);
			}
		}

		if (sOptions[i].Type == T_AIMKEYOPTION)
		{
			if (sOptions[i].Function == 0)
				DrawTextF(pContext, L"Right Mouse", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 1)
				DrawTextF(pContext, L"Left Mouse", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 2)
				DrawTextF(pContext, L"Shift", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 3)
				DrawTextF(pContext, L"Ctrl", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 4)
				DrawTextF(pContext, L"Alt", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 5)
				DrawTextF(pContext, L"Space", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 6)
				DrawTextF(pContext, L"X", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 7)
				DrawTextF(pContext, L"C", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);
		}

		if (sOptions[i].Type == T_MULTIOPTION)
		{
			if (sOptions[i].Function == 0)
				DrawTextF(pContext, L"0", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 1)
				DrawTextF(pContext, L"1", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 2)
				DrawTextF(pContext, L"2", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 3)
				DrawTextF(pContext, L"3", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 4)
				DrawTextF(pContext, L"4", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 5)
				DrawTextF(pContext, L"5", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 6)
				DrawTextF(pContext, L"6", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);

			if (sOptions[i].Function == 7)
				DrawTextF(pContext, L"7", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_On);
		}

		if (sOptions[i].Type == T_FOLDER)
		{
			if (sOptions[i].Function)
			{
				DrawTextF(pContext, L"Open", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_Folder);
			}
			else {
				DrawTextF(pContext, L"Closed", 14, sMenu.x + 150, sMenu.y + LineH*(i + 2), Color_Folder);
			}
		}
		DWORD Color = Color_Font;
		if (Cur_Pos == i)
			Color = Color_Current;
		DrawTextF(pContext, sOptions[i].Name, 14, sMenu.x + 6, sMenu.y + 1 + LineH*(i + 2), 0xFF2F4F4F);
		DrawTextF(pContext, sOptions[i].Name, 14, sMenu.x + 5, sMenu.y + LineH*(i + 2), Color);

	}
}

#define ORANGE 0xFF00BFFF
#define BLACK 0xFF000000
#define WHITE 0xFFFFFFFF
#define GREEN 0xFF00FF00 
#define RED 0xFFFF0000 
#define GRAY 0xFF2F4F4F


void Do_Menu()
{
	AddOption(L"Show Aimpoint", Item1, &Folder1);
	AddOption(L"Aimbot PvE", Item2, &Folder1);
	AddOption(L"Aimbot PvP", Item3, &Folder1);
	AddMultiOptionText(L"Aimkey", Item4, &Folder1);
	AddMultiOption(L"Aimsens", Item5, &Folder1);
	AddMultiOption(L"Aimfov", Item6, &Folder1);
	AddMultiOption(L"Aimheight", Item7, &Folder1);
	AddOption(L"Autoshoot", Item8, &Folder1);
}

//==========================================================================================================================

//w2s stuff
struct Vec2
{
	float x, y;
};

struct Vec3
{
	float x, y, z;
};

struct Vec4
{
	float x, y, z, w;
};

static Vec4 Vec4MulMat4x4(const Vec4& v, float(*mat4x4)[4])
{
	Vec4 o;
	
	o.x = v.x * mat4x4[0][0] + v.y * mat4x4[1][0] + v.z * mat4x4[2][0] + v.w * mat4x4[3][0];
	o.y = v.x * mat4x4[0][1] + v.y * mat4x4[1][1] + v.z * mat4x4[2][1] + v.w * mat4x4[3][1];
	o.z = v.x * mat4x4[0][2] + v.y * mat4x4[1][2] + v.z * mat4x4[2][2] + v.w * mat4x4[3][2];
	o.w = v.x * mat4x4[0][3] + v.y * mat4x4[1][3] + v.z * mat4x4[2][3] + v.w * mat4x4[3][3];

	return o;
}

static Vec4 Vec3MulMat4x4(const Vec3& v, float(*mat4x4)[4])
{
	Vec4 o;
	
	o.x = v.x * mat4x4[0][0] + v.y * mat4x4[1][0] + v.z * mat4x4[2][0] + mat4x4[3][0];
	o.y = v.x * mat4x4[0][1] + v.y * mat4x4[1][1] + v.z * mat4x4[2][1] + mat4x4[3][1];
	o.z = v.x * mat4x4[0][2] + v.y * mat4x4[1][2] + v.z * mat4x4[2][2] + mat4x4[3][2];
	o.w = v.x * mat4x4[0][3] + v.y * mat4x4[1][3] + v.z * mat4x4[2][3] + mat4x4[3][3];
	
	return o;
}

static Vec3 Vec3MulMat4x3(const Vec3& v, float(*mat4x3)[3])
{
	Vec3 o;
	o.x = v.x * mat4x3[0][0] + v.y * mat4x3[1][0] + v.z * mat4x3[2][0] + mat4x3[3][0];
	o.y = v.x * mat4x3[0][1] + v.y * mat4x3[1][1] + v.z * mat4x3[2][1] + mat4x3[3][1];
	o.z = v.x * mat4x3[0][2] + v.y * mat4x3[1][2] + v.z * mat4x3[2][2] + mat4x3[3][2];
	return o;
}

void MapBuffer(ID3D11Buffer* pStageBuffer, void** ppData, UINT* pByteWidth)
{
	D3D11_MAPPED_SUBRESOURCE subRes;
	HRESULT res = pContext->Map(pStageBuffer, 0, D3D11_MAP_READ, 0, &subRes);

	D3D11_BUFFER_DESC desc;
	pStageBuffer->GetDesc(&desc);

	if (FAILED(res))
	{
		//Log("Map stage buffer failed {%d} {%d} {%d} {%d} {%d}", (void*)pStageBuffer, desc.ByteWidth, desc.BindFlags, desc.CPUAccessFlags, desc.Usage);
	}

	*ppData = subRes.pData;

	if (pByteWidth)
		*pByteWidth = desc.ByteWidth;
}

void UnmapBuffer(ID3D11Buffer* pStageBuffer)
{
	pContext->Unmap(pStageBuffer, 0);
}

ID3D11Buffer* CopyBufferToCpu(ID3D11Buffer* pBuffer)
{
	D3D11_BUFFER_DESC CBDesc;
	pBuffer->GetDesc(&CBDesc);

	ID3D11Buffer* pStageBuffer = NULL;
	{ // create shadow buffer.
		D3D11_BUFFER_DESC desc;
		desc.BindFlags = 0;
		desc.ByteWidth = CBDesc.ByteWidth;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		desc.Usage = D3D11_USAGE_STAGING;

		if (FAILED(pDevice->CreateBuffer(&desc, NULL, &pStageBuffer)))
		{
			//Log("CreateBuffer failed when CopyBufferToCpu {%d}", CBDesc.ByteWidth);
		}
	}

	if (pStageBuffer != NULL)
		pContext->CopyResource(pStageBuffer, pBuffer);

	return pStageBuffer;
}

//get distance
float GetmDst(float Xx, float Yy, float xX, float yY)
{
	return sqrt((yY - Yy) * (yY - Yy) + (xX - Xx) * (xX - Xx));
}

struct AimEspInfo_t
{
	float vOutX, vOutY;
	float CrosshairDst;
};
std::vector<AimEspInfo_t>AimEspInfo;

//w2s
int WorldViewCBnum = 0;
int ProjCBnum = 0;
int matProjnum = 4;
//Game			WorldViewCBnum		ProjCBnum		matProjnum
//UT4 Alpha		2					1				16		
//Fortnite		2					1				16
//Outlast 		0					1				0 and 16
//Warframe		0					0				0 or 4
ID3D11Buffer* pWorldViewCB = nullptr;
ID3D11Buffer* pProjCB = nullptr;
ID3D11Buffer* m_pCurWorldViewCB = NULL;
ID3D11Buffer* m_pCurProjCB = NULL;
void AddModel(ID3D11DeviceContext* pContext)
{
	//Warning, this is NOT optimized:

	pContext->VSGetConstantBuffers(WorldViewCBnum, 1, &pWorldViewCB);//WorldViewCBnum

	if (m_pCurWorldViewCB == NULL && pWorldViewCB != NULL)
	m_pCurWorldViewCB = CopyBufferToCpu(pWorldViewCB);
	SAFE_RELEASE(pWorldViewCB);

	pContext->VSGetConstantBuffers(ProjCBnum, 1, &pProjCB);//ProjCBnum

	if (m_pCurProjCB == NULL && pProjCB != NULL)
	m_pCurProjCB = CopyBufferToCpu(pProjCB);
	SAFE_RELEASE(pProjCB);
	
	if (m_pCurWorldViewCB == NULL || m_pCurProjCB == NULL)//uncomment if a game is crashing
	return;
	
	float matWorldView[4][4];
	{
		float* worldview;
		MapBuffer(m_pCurWorldViewCB, (void**)&worldview, NULL);
		memcpy(matWorldView, &worldview[0], sizeof(matWorldView));
		//matWorldView[3][2] = matWorldView[3][2] + (aimheight * 10); //aimheight for body
		//matWorldView[3][3] = matWorldView[3][3] - countnum;
		UnmapBuffer(m_pCurWorldViewCB);
		SAFE_RELEASE(m_pCurWorldViewCB);
	}
	Vec3 v;
	Vec4 vWorldView = Vec3MulMat4x4(v, matWorldView);
	
	float matProj[4][4];
	{
		float *proj;
		MapBuffer(m_pCurProjCB, (void**)&proj, NULL);
		memcpy(matProj, &proj[matProjnum], sizeof(matProj));//matProjnum
		UnmapBuffer(m_pCurProjCB);
		SAFE_RELEASE(m_pCurProjCB);
	}
	Vec4 vWorldViewProj = Vec4MulMat4x4(vWorldView, matProj);


	Vec2 o;
	o.x = ScreenCenterX + ScreenCenterX * vWorldViewProj.x / vWorldViewProj.w;
	o.y = ScreenCenterY - ScreenCenterY * vWorldViewProj.y / vWorldViewProj.w;

	AimEspInfo_t pAimEspInfo = { static_cast<float>(o.x), static_cast<float>(o.y + (sOptions[6].Function * 10)) }; //aimheight for hp bars
	AimEspInfo.push_back(pAimEspInfo);
}

//==========================================================================================================================
