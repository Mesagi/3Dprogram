//�E�B���h�E�\����DirectX������
#include<Windows.h>
#include<tchar.h>
#include <DirectXMath.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include <d3dcompiler.h>
#include<vector>
#ifdef _DEBUG
#include<iostream>
#endif

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace std;
using namespace DirectX;

// @brief �R���\�[����ʂɃt�H�[�}�b�g�t���������\��
// @param format�t�H�[�}�b�g�i%d�Ƃ�%f�Ƃ��́j
// @param �ϒ�����
// @remarks ���̊֐��̓f�o�b�O�p�ł��B�f�o�b�O���ɂ������삵�܂���
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//�E�B���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY) {
		PostQuitMessage(0); //OS�ɑ΂��Ă���OS�͏I���Ɠ`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

const unsigned int window_width = 1280;
const unsigned int window_height = 720;

ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapchain = nullptr;
ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;
ID3D12CommandQueue* _cmdQueue = nullptr;

void EnableDebugLayer() {
	ID3D12Debug* debugLayer = nullptr;
	//if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)))) {
	//	debugLayer->EnableDebugLayer(); //  �f�o�b�O���C���[��L��������
	//	debugLayer->Release(); //  �L����������C���^�[�t�F�C�X���������
	//}
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	if (result == S_OK) {
		debugLayer->EnableDebugLayer(); //  �f�o�b�O���C���[��L��������
		debugLayer->Release(); //  �L����������C���^�[�t�F�C�X���������
	}
}



#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif
	DebugOutputFormatString("Show window test.");

	//�@�E�B���h�E�N���X�̐������o�^
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.style = 0;
	
	w.cbClsExtra = 0;
	w.cbWndExtra = 0;
	w.lpfnWndProc = (WNDPROC)WindowProcedure; // �R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T("DX12Sample");       // �A�v���P�[�V�����N���X���i�K���ł悢�j
	w.hInstance = GetModuleHandle(nullptr);   // �n���h���̎擾
	w.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 4);
	RegisterClassEx(&w); // �A�v���P�[�V�����N���X�i�E�B���h�E�N���X�̎w���OS�ɓ`����j
	RECT wrc = { 0, 0, window_width, window_height };  // �E�B���h�E�T�C�Y�����߂�

	// �֐����g���ăE�B���h�E�̃T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);// �E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName,      // �N���X���w��
		_T("DX12�e�X�g"),                                               // �^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,                               // �^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT,                                             // �\��x���W��OS�ɂ��C��
		CW_USEDEFAULT,                                             // �\��y���W��OS�ɂ��C�� 
		wrc.right - wrc.left,                                           // �E�B���h�E��
		wrc.bottom - wrc.top,                                       // �E�B���h�E��
		nullptr,                // �e�E�B���h�E�n���h��
		nullptr,                                 // ���j���[�n���h��  
		w.hInstance,            // �Ăяo���A�v���P�[�V�����n���h��
		nullptr);               // �ǉ��p�����[�^�[// �E�B���h�E�\��ShowWindow(hwnd, SW_SHOW);

#ifdef _DEBUG
//�f�o�b�O���C���[���I����
//�f�o�C�X�������O�ɂ���Ă����Ȃ��ƁA�f�o�C�X������ɂ���
//�f�o�C�X�����X�Ƃ��Ă��܂��̂Œ���
	EnableDebugLayer();
#endif

	// DirectX12�̓���
	//1.IDXGIFactory�𐶐�
	auto result = S_OK;
#ifdef _DEBUG
	result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
#else
	result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif
	if (result != S_OK) {
		DebugOutputFormatString("FAILED CreateDXGIFactory");
		return -1;
	}

	//2.VGA�A�_�v�^IDXGIAdapter�̔z���IDXGIFactory6������o��

	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		adapters.push_back(tmpAdapter);
	}

	//3.�g�������A�_�v�^��VGA�̃��[�J�[�őI��
	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc); // �A�_�v�^�[�̐����I�u�W�F�N�g�擾
		std::wstring strDesc = adesc.Description;    // �T�������A�_�v�^�[�̖��O���m�F
		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tmpAdapter = adpt;
			break;
		}
	}
	//4.ID3D12Device��I�񂾃A�_�v�^��p���ď���������������
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	D3D_FEATURE_LEVEL featureLevel;
	for (auto l : levels) {
		if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(&_dev)) == S_OK) {
			featureLevel = l;
			break;
		}
	}

	//�R�}���h����̏���
	// �R�}���h�A���P�[�^�[ID3D12CommandAllocator�𐶐�
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	if (result != S_OK) {
		DebugOutputFormatString("FAILED CreateCommandAllocator");
		return -1;
	}
	// �R�}���h���X�gID3D12GraphicsCommandList�𐶐�
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
	if (result != S_OK) {
		DebugOutputFormatString("FAILED CreateCommandList");
		return -1;
	}

	// �R�}���h�L���[D3D12_COMMAND_QUEUE_DESC�Ɋւ���ݒ���쐬 
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//�^�C���A�E�g�Ȃ�
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;//�v���C�I���e�B���Ɏw��Ȃ�
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//�����̓R�}���h���X�g�ƍ��킹�Ă�������
	// �R�}���h�L���[ID3D12CommandQueue�𐶐�
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));//�R�}���h�L���[����
	if (result != S_OK) {
		DebugOutputFormatString("FAILED CreateCommandQueue");
		return -1;
	}

	/////////////
	//�X���b�v�`�F�[��

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;// �o�b�N�o�b�t�@�[�͐L�яk�݉\
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;// �t���b�v��͑��₩�ɔj��
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;// ���Ɏw��Ȃ�
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;// �E�B���h�E�̃t���X�N���[���؂�ւ��\
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain);
	if (result != S_OK) {
		DebugOutputFormatString("FAILED CreateSwapChainForHwnd");
		return -1;
	}

	///////////
	// �f�B�X�N���v�^�[�q�[�v�̊m��

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//�����_�[�^�[�Q�b�g�r���[�Ȃ̂œ��RRTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;//�\���̂Q��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//���Ɏw��Ȃ�
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	if (result != S_OK) {
		DebugOutputFormatString("FAILED CreateDescriptorHeap");
		return -1;
	}

	////////////
	//�f�B�X�N���v�^�[�q�[�v���Ƀ����_�[�^�[�Q�b�g�r���[���쐬
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	if (result != S_OK) {
		DebugOutputFormatString("FAILED IDXGISwapChain4->GetDesc");
		return -1;
	}
	std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < swcDesc.BufferCount; ++i) {
		result = _swapchain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&_backBuffers[i]));	//�o�b�t�@�̈ʒu�̃n���h�������o��
		if (result != S_OK) {
			DebugOutputFormatString("FAILED IDXGISwapChain4->GetBuffer");
			return -1;
		}
		_dev->CreateRenderTargetView(_backBuffers[i], nullptr, handle);//RTV���f�B�X�N���v�^�[�q�[�v�ɍ쐬
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);// �f�B�X�N���v�^�̐擪�A�h���Y��RTV�̃T�C�Y���A���ւ��炸
	}

	// �t�F���X��p��
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	ShowWindow(hwnd, SW_SHOW);
	struct Vertex {
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};
	// ���W�̒�`
	Vertex vertices[] = {
		{{ 0.7f, 0.0f, 0.0f}, {0.0f,1.0f} },
		{{ 0.3f, 0.95f, 0.0f} ,{0.0f,0.0f} },
		{{ 0.3f, -0.95f, 0.0f} , {1.0f,1.0f} },//  �E��
		{{ -0.4f,  0.59f, 0.0f} ,{1.0f,0.0f} },//  ����
		{{ -0.4f,  -0.59f, 0.0f} ,{0.0f,1.0f} },
		 //  ���� 
		
		
		
	};


	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vertBuff = nullptr;
	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&vertBuff));

	if (result != S_OK) {
		DebugOutputFormatString("FAILED ID3D12Device::CreateCommittedResource");
		return -1;
	}
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (result != S_OK) {
		DebugOutputFormatString("FAILED ID3D12Resource::Map at transmitting Vertex Buffer");
		return -1;
	}
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	// ���_�o�b�t�@�r���[��p��
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress(); // �o�b�t�@�[�̉��z�A�h���X
	vbView.SizeInBytes = sizeof(vertices);      // �S�o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]); // 1���_������̃o�C�g��

	//�@Shader�p��Blob��p��
	ID3DBlob* _vsBlob = nullptr;	//�@���_�V�F�[�_�[
	ID3DBlob* _psBlob = nullptr;	// �s�N�Z���V�F�[�_�[
	ID3DBlob* errorBlob = nullptr;	//�@�G���[�i�[�p
	//�@���_�V�F�[�_�[���R���p�C������
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &_vsBlob, &errorBlob);
	if (SUCCEEDED(result))
	{	//�@���_�V�F�[�_�[�̃R���p�C�������A�s�N�Z���V�F�[�_�[�̃R���p�C��
		result = D3DCompileFromFile(
			L"BasicPixelShader.hlsl",
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"BasicPS",
			"ps_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0, &_psBlob, &errorBlob);
		if (FAILED(result)) {// �R���p�C���G���[�̏ꍇ
			if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
				::OutputDebugStringA("�V�F�[�_�[�t�@�C������������܂���");
			}
			else {
				std::string errstr;
				errstr.resize(errorBlob->GetBufferSize());
				std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
				errstr += "\n";
				OutputDebugStringA(errstr.c_str());
			}
			exit(1);
		}
	}
	else {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�V�F�[�_�[�t�@�C������������܂���");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}
	// ���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0,
        }
	};

	// �p�C�v���C���X�e�[�g�̐ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	gpipeline.pRootSignature = nullptr;
	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();
	// gpipeline.StreamOutput.NumEntries�ɂ��Ă͖��w��

//  �f�t�H���g�̃T���v���}�X�N��\���萔�i0xffffffff�j
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};

	//�ЂƂ܂����Z���Z�⃿�u�����f�B���O�͎g�p���Ȃ�
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//�ЂƂ܂��_�����Z�͎g�p���Ȃ�
	renderTargetBlendDesc.LogicOpEnable = false;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	// D3D12_RASTERIZER_DESC �̐ݒ�
	gpipeline.RasterizerState.MultisampleEnable = false;//�܂��A���`�F���͎g��Ȃ�
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//�J�����O���Ȃ�
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//���g��h��Ԃ�
	gpipeline.RasterizerState.DepthClipEnable = true;//�[�x�����̃N���b�s���O�͗L����

	gpipeline.RasterizerState.FrontCounterClockwise = false;
	gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipeline.RasterizerState.AntialiasedLineEnable = false;
	gpipeline.RasterizerState.ForcedSampleCount = 0;
	gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	// D3D12_DEPTH_STENCIL_DESC�@�[�x�X�e���V��   
	gpipeline.DepthStencilState.DepthEnable = false;
	gpipeline.DepthStencilState.StencilEnable = false;

	// �\�ߗp�ӂ������_���C�A�E�g��ݒ�
	gpipeline.InputLayout.pInputElementDescs = inputLayout;//���C�A�E�g�擪�A�h���X
	gpipeline.InputLayout.NumElements = _countof(inputLayout);//���C�A�E�g�z��

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED; //  �J�b�g�Ȃ�

	//  �O�p�`�ō\��
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//�@�����_�[�^�[�Q�b�g�̐ݒ�
	gpipeline.NumRenderTargets = 1; // ����1�̂�
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0�`1�ɐ��K�����ꂽRGBA

	gpipeline.SampleDesc.Count = 1;   // �T���v�����O��1�s�N�Z���ɂ�1
	gpipeline.SampleDesc.Quality = 0; // �N�I���e�B�͍Œ�

	// ���[�g�V�O�l�`���̍쐬
	ID3D12RootSignature* rootsignature = nullptr;
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(
		&rootSignatureDesc,									//���[�g�V�O�l�`���ݒ�        
		D3D_ROOT_SIGNATURE_VERSION_1_0,		// ���[�g�V�O�l�`���o�[�W����        
		&rootSigBlob,												// �V�F�[�_�[��������Ƃ��Ɠ���        
		&errorBlob);												// �G���[����������
	if (!SUCCEEDED(result)) {
		DebugOutputFormatString("FAILED SerializeRootSignature");
		return -1;
	}

	result = _dev->CreateRootSignature(
		0, // nodemask�B0�ł悢    
		rootSigBlob->GetBufferPointer(), // �V�F�[�_�[�̂Ƃ��Ɠ��l    
		rootSigBlob->GetBufferSize(),    // �V�F�[�_�[�̂Ƃ��Ɠ��l    
		IID_PPV_ARGS(&rootsignature));
	if (!SUCCEEDED(result)) {
		DebugOutputFormatString("FAILED ID3D12Device::CreateRootSignature");
		return -1;
	}

	rootSigBlob->Release(); // �s�v�ɂȂ����̂ŉ��
	gpipeline.pRootSignature = rootsignature;

	ID3D12PipelineState* _pipelinestate = nullptr;
	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));
	if (result != S_OK) {
		DebugOutputFormatString("FAILED ID3D12Device::CreateGraphicsPipelineState method");
		return -1;
	}

	//�@�r���[�|�[�g
	D3D12_VIEWPORT viewport = {};
	viewport.Width = window_width;   // �o�͐�̕��i�s�N�Z�����j
	viewport.Height = window_height; // �o�͐�̍����i�s�N�Z�����j
	viewport.TopLeftX = 0;    // �o�͐�̍�����WX
	viewport.TopLeftY = 0;    // �o�͐�̍�����WY
	viewport.MaxDepth = 1.0f; // �[�x�ő�l
	viewport.MinDepth = 0.0f; // �[�x�ŏ��l

	// �V�U�[��`
	D3D12_RECT scissorrect = {};
	scissorrect.top = 0;  // �؂蔲������W
	scissorrect.left = 0; //  �؂蔲�������W
	scissorrect.right = scissorrect.left + window_width;  //  �؂蔲���E���W
	scissorrect.bottom = scissorrect.top + window_height; //  �؂蔲�������W

	MSG msg = {};
	while (true) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}    //�A�v���P�[�V�������I���Ƃ���message��WM_QUIT�ɂȂ� 
		if (msg.message == WM_QUIT)
		{
			break;
		}

		/////////////
		// DirextX�ł̃t���[���`��
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();	//���݂̗L���ȃ����_�[�^�[�Q�b�g��ID���擾

		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		//�@�p�C�v���C���̐ݒ�
		_cmdList->SetPipelineState(_pipelinestate);

		// �����_�[�^�[�Q�b�g�̐ݒ�
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

		//��ʃN���A
		float clearColor[] = { 0.0f,1.0f,0.0f,0.0f };//12.5%�O���[
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		// �r���[�|�[�g�A�V�U�[�A���[�g�V�O�l�`���̐ݒ�
		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);
		_cmdList->SetGraphicsRootSignature(rootsignature);
		// �g�|���W
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		// ���_
		_cmdList->IASetVertexBuffers(0, 1, &vbView);
		//�@���_��`��
		_cmdList->DrawInstanced(5, 1, 0, 0);


		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &BarrierDesc);


		// ���߂̃N���[�Y
		_cmdList->Close();


		//  �R�}���h���X�g�̎��s
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		////�t�F���X�ŏ�����҂�
		_cmdQueue->Signal(_fence, ++_fenceVal);

		if (_fence->GetCompletedValue() != _fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		_cmdAllocator->Reset(); //  �L���[���N���A
		_cmdList->Reset(_cmdAllocator, nullptr); //  �ĂуR�}���h���X�g�����߂鏀��
		// �t���b�v
		_swapchain->Present(1, 0);
		/*
		if (result != S_OK) {
			DebugOutputFormatString("FAILED ID3D12CommandAllocator::Reset");
			return -1;
		}
		*/

	}
	
	//�����N���X�͎g��Ȃ��̂œo�^��������
	UnregisterClass(w.lpszClassName, w.hInstance);
	char c = getchar();
	return 0;
}