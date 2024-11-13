#include <Windows.h>
#include <d3dx9.h>

#pragma warning( disable : 4996 )
#pragma warning( default : 4996 )

LPDIRECT3D9             g_pD3D = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL;
LPD3DXMESH              g_pSphereMesh = NULL;

VOID SetupBowlingBallMaterial()
{
    D3DMATERIAL9 mtrl;
    ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
    mtrl.Diffuse.r = mtrl.Ambient.r = 0.7f;
    mtrl.Diffuse.g = mtrl.Ambient.g = 0.3f;
    mtrl.Diffuse.b = mtrl.Ambient.b = 0.3f;
    mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
    g_pd3dDevice->SetMaterial(&mtrl);
}

HRESULT InitD3D(HWND hWnd)
{
    if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
        return E_FAIL;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp, &g_pd3dDevice)))
    {
        return E_FAIL;
    }

    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
    return S_OK;
}

HRESULT InitGeometry()
{
    if (FAILED(D3DXCreateSphere(g_pd3dDevice, 1.0f, 20, 20, &g_pSphereMesh, NULL)))
        return E_FAIL;

    return S_OK;
}

VOID Cleanup()
{
    if (g_pSphereMesh != NULL)
        g_pSphereMesh->Release();

    if (g_pd3dDevice != NULL)
        g_pd3dDevice->Release();

    if (g_pD3D != NULL)
        g_pD3D->Release();
}
// ボールの初期位置を設定
D3DXVECTOR3 g_ballPosition(0.0f, 0.0f, 0.0f);
float g_rotationAngleX = 0.0f;
float g_rotationAngleZ = 0.0f;

VOID UpdateBallPosition()
{
    D3DXVECTOR3 previousPosition = g_ballPosition;
    // 左へ移動
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        g_ballPosition.x -= 0.05f;
        g_rotationAngleZ += 0.05f;
    }
    // 右へ移動
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        g_ballPosition.x += 0.05f;
        g_rotationAngleZ -= 0.05f;
    }
    // 奥へ移動
    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        g_ballPosition.z += 0.05f;
        g_rotationAngleX += 0.05f;
    }
    //手前に移動
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        g_ballPosition.z -= 0.05f;
        g_rotationAngleX -= 0.05f;
    }
}

VOID SetupMatrices()
{
    D3DXMATRIXA16 matRotationX, matRotationZ;
    D3DXMatrixRotationX(&matRotationX, g_rotationAngleX);
    D3DXMatrixRotationZ(&matRotationZ, g_rotationAngleZ);

    D3DXMATRIXA16 matTranslation;
    D3DXMatrixTranslation(&matTranslation, g_ballPosition.x, g_ballPosition.y, g_ballPosition.z);

    D3DXMATRIXA16 matWorld = matRotationX * matRotationZ * matTranslation;
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

    D3DXVECTOR3 vEyePt(0.0f, 2.0f, -6.0f);
    D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

    D3DVIEWPORT9 viewport;
    g_pd3dDevice->GetViewport(&viewport);
    float aspectRatio = (float)viewport.Width / (float)viewport.Height;
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, aspectRatio, 1.0f, 100.0f);
    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

VOID SetupLights()
{
    D3DLIGHT9 light;
    ZeroMemory(&light, sizeof(D3DLIGHT9));
    light.Type = D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r = 0.8f;
    light.Diffuse.g = 0.8f;
    light.Diffuse.b = 0.8f;
    D3DXVECTOR3 vecDir(0.0f, -1.0f, 1.0f);
    D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);
    light.Range = 1000.0f;

    g_pd3dDevice->SetLight(0, &light);
    g_pd3dDevice->LightEnable(0, TRUE);
}

VOID Render()
{
    UpdateBallPosition();
    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(200, 200, 200), 1.0f, 0);

    if (SUCCEEDED(g_pd3dDevice->BeginScene()))
    {
        SetupLights();
        SetupBowlingBallMaterial();
        SetupMatrices();

        g_pSphereMesh->DrawSubset(0);

        g_pd3dDevice->EndScene();
    }
    g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_DESTROY)
    {
        Cleanup();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, INT)
{
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
        GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
        L"D3D Tutorial", NULL
    };
    RegisterClassEx(&wc);

    HWND hWnd = CreateWindow(L"D3D Tutorial", L"Bowling Ball",
        WS_OVERLAPPEDWINDOW, 100, 100, 800, 600,
        NULL, NULL, wc.hInstance, NULL);

    if (SUCCEEDED(InitD3D(hWnd)))
    {
        if (SUCCEEDED(InitGeometry()))
        {
            ShowWindow(hWnd, SW_SHOWDEFAULT);
            UpdateWindow(hWnd);
            MSG msg;
            ZeroMemory(&msg, sizeof(msg));
            while (msg.message != WM_QUIT)
            {
                if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                else
                    Render();
            }
        }
    }

    UnregisterClass(L"D3D Tutorial", wc.hInstance);
    return 0;
}
