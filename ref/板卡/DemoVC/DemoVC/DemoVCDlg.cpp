
// DemoVCDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DemoVC.h"
#include "DemoVCDlg.h"
#include "afxdialogex.h"

#include "GAS_N.h"
#pragma comment(lib,"..\\Debug\\gas.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TCrdData LookAheadBuf[FROCAST_LEN];                 //前瞻缓冲区

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDemoVCDlg 对话框




CDemoVCDlg::CDemoVCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDemoVCDlg::IDD, pParent)
	, m_lTargetPos(0)
	, m_lOrginX(0)
	, m_lOrginY(0)
	, m_lOrginZ(0)
	, m_lOrginX2(0)
	, m_lOrginY2(0)
	, m_lOrginZ2(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDemoVCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ComboBoxAxis);
	DDX_Text(pDX, IDC_EDIT1, m_lTargetPos);
	DDX_Control(pDX, IDC_COMBO_CARD_SEL, m_ComboBoxCardSel);
	DDX_Control(pDX, IDC_SLIDER_OVER_RIDE, m_SliderCrdVel);
	DDX_Control(pDX, IDC_SLIDER_OVER_RIDE2, m_SliderCrdVel2);
	DDX_Text(pDX, IDC_EDIT_ORGIN_X, m_lOrginX);
	DDX_Text(pDX, IDC_EDIT_ORGIN_Y, m_lOrginY);
	DDX_Text(pDX, IDC_EDIT_ORGIN_Z, m_lOrginZ);
	DDX_Text(pDX, IDC_EDIT_ORGIN_X2, m_lOrginX2);
	DDX_Text(pDX, IDC_EDIT_ORGIN_Y2, m_lOrginY2);
	DDX_Text(pDX, IDC_EDIT_ORGIN_Z2, m_lOrginZ2);
}

BEGIN_MESSAGE_MAP(CDemoVCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON4, &CDemoVCDlg::OnBnClickedButton4)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON7, &CDemoVCDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON2, &CDemoVCDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CDemoVCDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON1, &CDemoVCDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK_NO_ENCODE, &CDemoVCDlg::OnBnClickedCheckNoEncode)
	ON_BN_CLICKED(IDC_BUTTON_CLR_STS, &CDemoVCDlg::OnBnClickedButtonClrSts)
	ON_BN_CLICKED(IDC_BUTTON_AXIS_ON, &CDemoVCDlg::OnBnClickedButtonAxisOn)
	ON_BN_CLICKED(IDC_BUTTON6, &CDemoVCDlg::OnBnClickedButton6)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDemoVCDlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_RADIO_INPUT_1, &CDemoVCDlg::OnBnClickedRadioInput1)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_1, &CDemoVCDlg::OnBnClickedButtonOutPut1)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_2, &CDemoVCDlg::OnBnClickedButtonOutPut2)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_3, &CDemoVCDlg::OnBnClickedButtonOutPut3)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_4, &CDemoVCDlg::OnBnClickedButtonOutPut4)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_5, &CDemoVCDlg::OnBnClickedButtonOutPut5)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_6, &CDemoVCDlg::OnBnClickedButtonOutPut6)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_7, &CDemoVCDlg::OnBnClickedButtonOutPut7)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_8, &CDemoVCDlg::OnBnClickedButtonOutPut8)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_9, &CDemoVCDlg::OnBnClickedButtonOutPut9)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_10, &CDemoVCDlg::OnBnClickedButtonOutPut10)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_11, &CDemoVCDlg::OnBnClickedButtonOutPut11)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_12, &CDemoVCDlg::OnBnClickedButtonOutPut12)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_13, &CDemoVCDlg::OnBnClickedButtonOutPut13)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_14, &CDemoVCDlg::OnBnClickedButtonOutPut14)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_15, &CDemoVCDlg::OnBnClickedButtonOutPut15)
	ON_BN_CLICKED(IDC_BUTTON_OUT_PUT_16, &CDemoVCDlg::OnBnClickedButtonOutPut16)
	ON_BN_CLICKED(IDC_CHECK_POS_LIM_ENABLE, &CDemoVCDlg::OnBnClickedCheckPosLimEnable)
	ON_BN_CLICKED(IDC_CHECK_NEG_LIM_ENABLE, &CDemoVCDlg::OnBnClickedCheckNegLimEnable)
	ON_BN_CLICKED(IDC_BUTTON9, &CDemoVCDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON8, &CDemoVCDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON_JOG_POS, &CDemoVCDlg::OnBnClickedButtonJogPos)
	ON_BN_CLICKED(IDC_BUTTON_JOG_NEG, &CDemoVCDlg::OnBnClickedButtonJogNeg)
	ON_BN_CLICKED(IDC_BUTTON_SET_CRD_PRM, &CDemoVCDlg::OnBnClickedButtonSetCrdPrm)
	ON_BN_CLICKED(IDC_BUTTON10, &CDemoVCDlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON11, &CDemoVCDlg::OnBnClickedButton11)
	ON_BN_CLICKED(IDC_BUTTON12, &CDemoVCDlg::OnBnClickedButton12)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_OVER_RIDE, &CDemoVCDlg::OnNMReleasedcaptureSliderOverRide)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_OVER_RIDE, &CDemoVCDlg::OnNMCustomdrawSliderOverRide)
	ON_BN_CLICKED(IDC_BUTTON_SET_CRD_PRM2, &CDemoVCDlg::OnBnClickedButtonSetCrdPrm2)
	ON_BN_CLICKED(IDC_BUTTON13, &CDemoVCDlg::OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON14, &CDemoVCDlg::OnBnClickedButton14)
	ON_BN_CLICKED(IDC_BUTTON15, &CDemoVCDlg::OnBnClickedButton15)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_OVER_RIDE2, &CDemoVCDlg::OnNMReleasedcaptureSliderOverRide2)
END_MESSAGE_MAP()


// CDemoVCDlg 消息处理程序

BOOL CDemoVCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_ComboBoxAxis.AddString("轴1（Axis1）");
	m_ComboBoxAxis.AddString("轴2（Axis2）");
	m_ComboBoxAxis.AddString("轴3（Axis3）");
	m_ComboBoxAxis.AddString("轴4（Axis4）");
	m_ComboBoxAxis.AddString("轴5（Axis5）");
	m_ComboBoxAxis.AddString("轴6（Axis6）");
	m_ComboBoxAxis.AddString("轴7（Axis7）");
	m_ComboBoxAxis.AddString("轴8（Axis8）");

	m_ComboBoxAxis.SetCurSel(0);

	m_ComboBoxCardSel.AddString("主卡");
	m_ComboBoxCardSel.AddString("扩展卡1");
	m_ComboBoxCardSel.AddString("扩展卡2");
	m_ComboBoxCardSel.AddString("扩展卡3");
	m_ComboBoxCardSel.AddString("扩展卡4");
	m_ComboBoxCardSel.AddString("扩展卡5");
	m_ComboBoxCardSel.AddString("扩展卡6");
	m_ComboBoxCardSel.AddString("扩展卡7");
	m_ComboBoxCardSel.AddString("扩展卡8");
	m_ComboBoxCardSel.SetCurSel(0);

	//设置初始位置
	m_SliderCrdVel.SetPos(50);  
	m_SliderCrdVel2.SetPos(50);  
	m_SliderCrdVel.SetRange(0,120);
	m_SliderCrdVel2.SetRange(0,120);
	//设置在控件上单击时滑块移动步长
	m_SliderCrdVel.SetPageSize(5);
	m_SliderCrdVel2.SetPageSize(5);

	SetTimer(1,100,NULL);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CDemoVCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDemoVCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDemoVCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CDemoVCDlg::OnBnClickedButton4()
{
	int iAxisNum = m_ComboBoxAxis.GetCurSel()+1;
	GA_ZeroPos(iAxisNum);
}


void CDemoVCDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int iRes = 0;
	long lSts = 0;
	double dPrfPos = 0;
	double dEncPos = 0;
	unsigned long lValue = 0;
	CString strText;

	int iAxisNum = m_ComboBoxAxis.GetCurSel()+1;
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();

	iRes = GA_GetPrfPos(iAxisNum,&dPrfPos,1);
	iRes += GA_GetEncPos(iAxisNum,&dEncPos,1);
	iRes += GA_GetSts(iAxisNum,&lSts);

	strText.Format("%.3f",dPrfPos);
	GetDlgItem(IDC_STATIC_PRF_POS)->SetWindowText(strText);

	strText.Format("%.3f",dEncPos);
	GetDlgItem(IDC_STATIC_ENC_POS)->SetWindowText(strText);


	long nValue = 0;
	GA_GetDiRaw(MC_MPG,&nValue);

//status
	if(lSts & AXIS_STATUS_SV_ALARM)
	{
		((CButton*)GetDlgItem(IDC_RADIO_ALARM))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_ALARM))->SetCheck(false);
	}

	if(lSts & AXIS_STATUS_RUNNING)
	{
		((CButton*)GetDlgItem(IDC_RADIO_RUNNING))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_RUNNING))->SetCheck(false);
	}

	//正极限位
	if(lSts & AXIS_STATUS_POS_HARD_LIMIT)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_STATUS))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_STATUS))->SetCheck(false);
	}

	//负极限位
	if(lSts & AXIS_STATUS_NEG_HARD_LIMIT)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_STATUS))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_STATUS))->SetCheck(false);
	}

	
//input
	iRes = GA_GetExtDiValue(iCardIndex,&lValue,1);

	if(lValue & 0X0001)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_1))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_1))->SetCheck(false);
	}

	if(lValue & 0X0002)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_2))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_2))->SetCheck(false);
	}

	if(lValue & 0X0004)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_3))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_3))->SetCheck(false);
	}

	if(lValue & 0X0008)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_4))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_4))->SetCheck(false);
	}

	if(lValue & 0X0010)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_5))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_5))->SetCheck(false);
	}

	if(lValue & 0X0020)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_6))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_6))->SetCheck(false);
	}

	if(lValue & 0X0040)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_7))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_7))->SetCheck(false);
	}

	if(lValue & 0X0080)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_8))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_8))->SetCheck(false);
	}

	if(lValue & 0X0100)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_9))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_9))->SetCheck(false);
	}

	if(lValue & 0X0200)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_10))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_10))->SetCheck(false);
	}

	if(lValue & 0X0400)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_11))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_11))->SetCheck(false);
	}

	if(lValue & 0X0800)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_12))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_12))->SetCheck(false);
	}

	if(lValue & 0X1000)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_13))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_13))->SetCheck(false);
	}

	if(lValue & 0X2000)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_14))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_14))->SetCheck(false);
	}

	if(lValue & 0X4000)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_15))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_15))->SetCheck(false);
	}

	if(lValue & 0X8000)
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_16))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_INPUT_16))->SetCheck(false);
	}
//Lim-
	iRes = GA_GetDiRaw(MC_LIMIT_NEGATIVE,(long*)&lValue);

	if(lValue & 0X0001)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_1))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_1))->SetCheck(false);
	}
	if(lValue & 0X0002)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_2))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_2))->SetCheck(false);
	}
	if(lValue & 0X0004)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_3))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_3))->SetCheck(false);
	}
	if(lValue & 0X0008)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_4))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_4))->SetCheck(false);
	}
	if(lValue & 0X0010)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_5))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_5))->SetCheck(false);
	}
	if(lValue & 0X0020)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_6))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_6))->SetCheck(false);
	}
	if(lValue & 0X0040)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_7))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_7))->SetCheck(false);
	}
	if(lValue & 0X0080)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_8))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMN_8))->SetCheck(false);
	}
//Lim+
	iRes = GA_GetDiRaw(MC_LIMIT_POSITIVE,(long*)&lValue);

	if(lValue & 0X0001)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_1))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_1))->SetCheck(false);
	}

	if(lValue & 0X0002)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_2))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_2))->SetCheck(false);
	}
	if(lValue & 0X0004)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_3))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_3))->SetCheck(false);
	}
	if(lValue & 0X0008)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_4))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_4))->SetCheck(false);
	}
	if(lValue & 0X0010)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_5))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_5))->SetCheck(false);
	}
	if(lValue & 0X0020)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_6))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_6))->SetCheck(false);
	}
	if(lValue & 0X0040)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_7))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_7))->SetCheck(false);
	}
	if(lValue & 0X0080)
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_8))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_LIMP_8))->SetCheck(false);
	}

	long lCrdSpace = 0;

//---------------------------------------------------通道1---------------------------------------------------------------
//获取坐标系剩余空间
	iRes = GA_CrdSpace(1,&lCrdSpace,0);

	strText.Format("板卡插补缓冲区剩余空间：%d",lCrdSpace);
	GetDlgItem(IDC_STATIC_CRD_SPACE)->SetWindowText(strText);

//获取前瞻缓冲区剩余空间
	iRes = GA_GetLookAheadSpace(1,&lCrdSpace,0);

	strText.Format("板卡前瞻缓冲区剩余空间：%d",lCrdSpace);
	GetDlgItem(IDC_STATIC_LOOK_AHEAD_SPACE)->SetWindowText(strText);

//获取坐标系位置

	double dPos[8];
	iRes = GA_GetCrdPos(1,dPos);

	strText.Format("X:%.3f",dPos[0]);
	GetDlgItem(IDC_STATIC_POS_x)->SetWindowText(strText);

	strText.Format("Y:%.3f",dPos[1]);
	GetDlgItem(IDC_STATIC_POS_Y)->SetWindowText(strText);


	strText.Format("Z:%.3f",dPos[2]);
	GetDlgItem(IDC_STATIC_POS_Z)->SetWindowText(strText);

	short nCrdRuning = 0;
	iRes = GA_CrdStatus(1,&nCrdRuning,NULL);

	if(nCrdRuning & CRDSYS_STATUS_PROG_RUN)
	{
		((CButton*)GetDlgItem(IDC_RADIO_CRD_RUNNING))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_CRD_RUNNING))->SetCheck(false);
	}

	if(nCrdRuning & CRDSYS_STATUS_FIFO_FINISH_0)
	{
		((CButton*)GetDlgItem(IDC_RADIO_CRD_BUFFER_FINISH))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_CRD_BUFFER_FINISH))->SetCheck(false);
	}

	long lSegNum = 0;
	GA_GetUserSegNum(1,&lSegNum);

	strText.Format("运行行号：%d",lSegNum);
	GetDlgItem(IDC_STATIC_CUR_SEG_NUM)->SetWindowText(strText);


//---------------------------------------------------通道2---------------------------------------------------------------
//获取坐标系剩余空间
	iRes = GA_CrdSpace(2,&lCrdSpace,0);

	strText.Format("板卡插补缓冲区剩余空间：%d",lCrdSpace);
	GetDlgItem(IDC_STATIC_CRD_SPACE3)->SetWindowText(strText);

//获取前瞻缓冲区剩余空间
	iRes = GA_GetLookAheadSpace(2,&lCrdSpace,0);

	strText.Format("板卡前瞻缓冲区剩余空间：%d",lCrdSpace);
	GetDlgItem(IDC_STATIC_LOOK_AHEAD_SPACE2)->SetWindowText(strText);

//获取坐标系位置
	memset(dPos,0,sizeof(dPos));
	iRes = GA_GetCrdPos(2,dPos);

	strText.Format("X:%.3f",dPos[0]);
	GetDlgItem(IDC_STATIC_POS_x2)->SetWindowText(strText);

	strText.Format("Y:%.3f",dPos[1]);
	GetDlgItem(IDC_STATIC_POS_Y2)->SetWindowText(strText);


	strText.Format("Z:%.3f",dPos[2]);
	GetDlgItem(IDC_STATIC_POS_Z2)->SetWindowText(strText);

	iRes = GA_CrdStatus(2,&nCrdRuning,NULL);

	if(nCrdRuning & CRDSYS_STATUS_PROG_RUN)
	{
		((CButton*)GetDlgItem(IDC_RADIO_CRD_RUNNING2))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_CRD_RUNNING2))->SetCheck(false);
	}

	if(nCrdRuning & CRDSYS_STATUS_FIFO_FINISH_0)
	{
		((CButton*)GetDlgItem(IDC_RADIO_CRD_BUFFER_FINISH2))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_CRD_BUFFER_FINISH2))->SetCheck(false);
	}

	GA_GetUserSegNum(2,&lSegNum);

	strText.Format("运行行号：%d",lSegNum);
	GetDlgItem(IDC_STATIC_CUR_SEG_NUM2)->SetWindowText(strText);
	CDialogEx::OnTimer(nIDEvent);

}


void CDemoVCDlg::OnBnClickedButton7()
{
	int iRes = 0;

	//用串口方式打开
	iRes = GA_Open(1);

	//如果串口方式打开失败，尝试用网口打开（该例程为串口网口通用例程，所以这样做，如果确认是串口卡，该段代码可以不要）
	if(iRes)
	{
		iRes = GA_Open(0,"192.168.0.200");
	}
	//iRes += GA_Reset();

	if(iRes)
	{
		MessageBox("Open Card Fail,Please turn off wifi ,check PC IP address or connection!");
	}
	else
	{
		MessageBox("Open Card Successful!");
		OnCbnSelchangeCombo1();
	}
}


void CDemoVCDlg::OnBnClickedButton2()
{
	
}


void CDemoVCDlg::OnBnClickedButton3()
{
	GA_Stop(0XFFFF,0XFFFF);
}


void CDemoVCDlg::OnBnClickedButton1()
{
	
}


void CDemoVCDlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CDemoVCDlg::OnBnClickedCheckNoEncode()
{
	int iRes = 0;
	int iAxisNum = m_ComboBoxAxis.GetCurSel()+1;

	if(((CButton*)GetDlgItem(IDC_CHECK_NO_ENCODE))->GetCheck())
	{
		iRes = GA_EncOn(iAxisNum);
		((CButton*)GetDlgItem(IDC_CHECK_NO_ENCODE))->SetCheck(false);
	}
	else
	{
		iRes = GA_EncOff(iAxisNum);
		((CButton*)GetDlgItem(IDC_CHECK_NO_ENCODE))->SetCheck(true);
	}
}

void CDemoVCDlg::OnBnClickedCheckPosLimEnable()
{
	int iRes = 0;
	short nPosLimOnOff = 0;
	short nNegLimOnOff = 0;

	int iAxisNum = m_ComboBoxAxis.GetCurSel()+1;

	//正限位
	if(((CButton*)GetDlgItem(IDC_CHECK_POS_LIM_ENABLE))->GetCheck())
	{
		iRes = GA_LmtsOn(iAxisNum,MC_LIMIT_POSITIVE);

		((CButton*)GetDlgItem(IDC_CHECK_POS_LIM_ENABLE))->SetCheck(true);
	}
	else
	{
		iRes = GA_LmtsOff(iAxisNum,MC_LIMIT_POSITIVE);

		((CButton*)GetDlgItem(IDC_CHECK_POS_LIM_ENABLE))->SetCheck(false);
	}
}


void CDemoVCDlg::OnBnClickedCheckNegLimEnable()
{
	int iRes = 0;
	short nPosLimOnOff = 0;
	short nNegLimOnOff = 0;

	int iAxisNum = m_ComboBoxAxis.GetCurSel()+1;

	//正限位
	if(((CButton*)GetDlgItem(IDC_CHECK_NEG_LIM_ENABLE))->GetCheck())
	{
		iRes = GA_LmtsOn(iAxisNum,MC_LIMIT_NEGATIVE);

		((CButton*)GetDlgItem(IDC_CHECK_NEG_LIM_ENABLE))->SetCheck(true);
	}
	else
	{
		iRes = GA_LmtsOff(iAxisNum,MC_LIMIT_NEGATIVE);

		((CButton*)GetDlgItem(IDC_CHECK_NEG_LIM_ENABLE))->SetCheck(false);
	}
}


void CDemoVCDlg::OnBnClickedButtonClrSts()
{
	int iRes = 0;
	int iAxisNum = m_ComboBoxAxis.GetCurSel()+1;

	iRes = GA_ClrSts(iAxisNum);
}


void CDemoVCDlg::OnBnClickedButtonAxisOn()
{
	int iRes = 0;
	CString strText;
	int iAxisNum = m_ComboBoxAxis.GetCurSel()+1;

	GetDlgItemText(IDC_BUTTON_AXIS_ON,strText);
	if (strText == _T("伺服使能"))
	{
		iRes = GA_AxisOn(iAxisNum);
		GetDlgItem(IDC_BUTTON_AXIS_ON)->SetWindowText(_T("断开使能"));
	}
	else
	{
		iRes = GA_AxisOff(iAxisNum);
		GetDlgItem(IDC_BUTTON_AXIS_ON)->SetWindowText(_T("伺服使能"));
	}
}


void CDemoVCDlg::OnBnClickedButton6()
{
	int iRes = 0;
	int iAxisNum = m_ComboBoxAxis.GetCurSel()+1;

	UpdateData(true);

	TTrapPrm TrapPrm;

	TrapPrm.acc = 0.1;
	TrapPrm.dec = 0.1;
	TrapPrm.smoothTime = 0;
	TrapPrm.velStart = 0;

	iRes = GA_PrfTrap(iAxisNum);
	iRes += GA_SetTrapPrm(iAxisNum,&TrapPrm);

	iRes += GA_SetPos(iAxisNum,m_lTargetPos);
	iRes = GA_SetVel(iAxisNum,5);

	iRes += GA_Update(0X0001 << (iAxisNum-1));
}


void CDemoVCDlg::OnCbnSelchangeCombo1()
{
	int iRes = 0;
	short nEncOnOff = 0;
	short nPosLimOnOff = 0;
	short nNegLimOfOff = 0;
	int iAxisNum = m_ComboBoxAxis.GetCurSel()+1;


	iRes = GA_GetEncOnOff(iAxisNum,&nEncOnOff);

	if(nEncOnOff)
	{
		((CButton*)GetDlgItem(IDC_CHECK_NO_ENCODE))->SetCheck(false);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHECK_NO_ENCODE))->SetCheck(true);
	}

	iRes = GA_GetLmtsOnOff(iAxisNum,&nPosLimOnOff,&nNegLimOfOff);

	if(nPosLimOnOff)
	{
		((CButton*)GetDlgItem(IDC_CHECK_POS_LIM_ENABLE))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHECK_POS_LIM_ENABLE))->SetCheck(false);
	}

	if(nNegLimOfOff)
	{
		((CButton*)GetDlgItem(IDC_CHECK_NEG_LIM_ENABLE))->SetCheck(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHECK_NEG_LIM_ENABLE))->SetCheck(false);
	}
}


void CDemoVCDlg::OnBnClickedRadioInput1()
{

}


void CDemoVCDlg::OnBnClickedButtonOutPut1()
{
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_1)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,0,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_1)->SetWindowText("OutPut1 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,0,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_1)->SetWindowText("OutPut1 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut2()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_2)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,1,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_2)->SetWindowText("OutPut2 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,1,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_2)->SetWindowText("OutPut2 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut3()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_3)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,2,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_3)->SetWindowText("OutPut3 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,2,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_3)->SetWindowText("OutPut3 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut4()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_4)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,3,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_4)->SetWindowText("OutPut4 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,3,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_4)->SetWindowText("OutPut4 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut5()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_5)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,4,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_5)->SetWindowText("OutPut5 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,4,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_5)->SetWindowText("OutPut5 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut6()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_6)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,5,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_6)->SetWindowText("OutPut6 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,5,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_6)->SetWindowText("OutPut6 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut7()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_7)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,6,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_7)->SetWindowText("OutPut7 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,6,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_7)->SetWindowText("OutPut7 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut8()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_8)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,7,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_8)->SetWindowText("OutPut8 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,7,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_8)->SetWindowText("OutPut8 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut9()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_9)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,8,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_9)->SetWindowText("OutPut9 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,8,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_9)->SetWindowText("OutPut9 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut10()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_10)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,9,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_10)->SetWindowText("OutPut10 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,9,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_10)->SetWindowText("OutPut10 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut11()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_11)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,10,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_11)->SetWindowText("OutPut11 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,10,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_11)->SetWindowText("OutPut11 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut12()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_12)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,11,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_12)->SetWindowText("OutPut12 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,11,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_12)->SetWindowText("OutPut12 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut13()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_13)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,12,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_13)->SetWindowText("OutPut13 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,12,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_13)->SetWindowText("OutPut13 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut14()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_14)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,13,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_14)->SetWindowText("OutPut14 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,13,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_14)->SetWindowText("OutPut14 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut15()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_15)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,14,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_15)->SetWindowText("OutPut15 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,14,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_15)->SetWindowText("OutPut15 ON");
	}
}


void CDemoVCDlg::OnBnClickedButtonOutPut16()
{
	// TODO: 在此添加控件通知处理程序代码
	int iCardIndex = m_ComboBoxCardSel.GetCurSel();
	CString strText;

	GetDlgItem(IDC_BUTTON_OUT_PUT_16)->GetWindowText(strText);

	if(strstr(strText,"ON"))
	{
		GA_SetExtDoBit(iCardIndex,15,1);
		GetDlgItem(IDC_BUTTON_OUT_PUT_16)->SetWindowText("OutPut16 OFF");
	}
	else
	{
		GA_SetExtDoBit(iCardIndex,15,0);
		GetDlgItem(IDC_BUTTON_OUT_PUT_16)->SetWindowText("OutPut16 ON");
	}
}





void CDemoVCDlg::OnBnClickedButton9()
{
	GA_Close();
}


void CDemoVCDlg::OnBnClickedButton8()
{
	GA_Reset();
}


void CDemoVCDlg::OnBnClickedButton5()
{
	int iRes = 0;
	TJogPrm m_JogPrm;

	int iAxisNum = m_ComboBoxAxis.GetCurSel()+1;

	m_JogPrm.dAcc = 0.1;
	m_JogPrm.dDec = 0.1;
	m_JogPrm.dSmooth = 0;

	iRes = GA_PrfJog(iAxisNum);

	iRes += GA_SetJogPrm(iAxisNum,&m_JogPrm);

	iRes += GA_SetVel(iAxisNum,50);

	iRes += GA_Update(0X0001 << (iAxisNum-1));

	if(0 == iRes)
	{
		TRACE("正向连续移动2......\r\n");
	}
}


BOOL CDemoVCDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	switch(pMsg->message)
	{
	case WM_LBUTTONDOWN:

		UpdateData(TRUE);
		//负向运动按键按下
		if(pMsg->hwnd == GetDlgItem(IDC_BUTTON_JOG_NEG)->m_hWnd)
		{
			int iRes = 0;
			TJogPrm m_JogPrm;

			int iAxisNum = m_ComboBoxAxis.GetCurSel()+1;

			m_JogPrm.dAcc = 0.1;
			m_JogPrm.dDec = 0.1;
			m_JogPrm.dSmooth = 0;

			iRes = GA_PrfJog(iAxisNum);

			iRes += GA_SetJogPrm(iAxisNum,&m_JogPrm);

			iRes += GA_SetVel(iAxisNum,-20);

			iRes += GA_Update(0X0001 << (iAxisNum-1));

			if(0 == iRes)
			{
				TRACE("负向连续移动......\r\n");
			}
		}
		//正向运动按键按下
		if(pMsg->hwnd == GetDlgItem(IDC_BUTTON_JOG_POS)->m_hWnd)
		{
			int iRes = 0;
			TJogPrm m_JogPrm;

			int iAxisNum = m_ComboBoxAxis.GetCurSel()+1;

			m_JogPrm.dAcc = 0.1;
			m_JogPrm.dDec = 0.1;
			m_JogPrm.dSmooth = 0;

			iRes = GA_PrfJog(iAxisNum);

			iRes += GA_SetJogPrm(iAxisNum,&m_JogPrm);

			iRes += GA_SetVel(iAxisNum,20);

			iRes += GA_Update(0X0001 << (iAxisNum-1));

			if(0 == iRes)
			{
				TRACE("正向连续移动......\r\n");
			}
		}
		break;
	case WM_LBUTTONUP:
		//负向连续运动按键抬起
		if(pMsg->hwnd == GetDlgItem(IDC_BUTTON_JOG_NEG)->m_hWnd)
		{
			//停止运动
			GA_Stop(0XFFFF,0XFFFF);
		}
		//正向连续运动按键抬起
		if(pMsg->hwnd == GetDlgItem(IDC_BUTTON_JOG_POS)->m_hWnd)
		{
			//停止运动
			GA_Stop(0XFFFF,0XFFFF);
		}
		break;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDemoVCDlg::OnBnClickedButtonJogPos()
{
	//正向连续旋转代码在PreTranslateMessage函数里面
	//可以在本文档搜索PreTranslateMessage函数
}


void CDemoVCDlg::OnBnClickedButtonJogNeg()
{
	//反向连续旋转代码在PreTranslateMessage函数里面
	//可以在本文档搜索PreTranslateMessage函数
}


void CDemoVCDlg::OnBnClickedButtonSetCrdPrm()
{
	int iRes = 0;

	UpdateData(true);

//---------------------------------------建立坐标系------------------------------------------------
	//坐标系参数初始化
	TCrdPrm crdPrm;
	memset(&crdPrm,0,sizeof(crdPrm));

	//设置坐标系最小匀速时间为0
	crdPrm.evenTime = 0;

	//设置原点坐标值标志,0:默认当前规划位置为原点位置;1:用户指定原点位置
	crdPrm.setOriginFlag = 1;

	//设置坐标系原点
	
	crdPrm.originPos[0] = m_lOrginX;
	crdPrm.originPos[1] = m_lOrginY;
	crdPrm.originPos[2] = m_lOrginZ;
	

	//设置坐标系维数为3
	crdPrm.dimension = 3;

	//设置坐标系规划对应轴通道，通道1对应轴1，通道2对应轴3，通道3对应轴5
	crdPrm.profile[0] = 1;
	crdPrm.profile[1] = 2;
	crdPrm.profile[2] = 3;


	//设置坐标系最大加速度为0.2脉冲/毫秒^2
	crdPrm.synAccMax = 0.2;
	//设置坐标系最大速度为100脉冲/毫秒
	crdPrm.synVelMax = 100;

//------------------------------------------------------------------------------------------------------------------
	//设置坐标系参数，建立坐标系
	//iRes = GA_SetCrdPrm(1,&crdPrm);

	GA_SetCrdPrmSingle(1,crdPrm.dimension,crdPrm.profile,crdPrm.synVelMax,crdPrm.synAccMax,crdPrm.evenTime,crdPrm.setOriginFlag,crdPrm.originPos);
//------------------------------------------------------------------------------------------------------------------
	if(0 == iRes)
	{
		AfxMessageBox("建立坐标系成功！");
	}
	else
	{
		AfxMessageBox("建立坐标系失败！确认是否重复建立坐标系或者尚未停止当前坐标系！");
		return;
	}
//------------------------------------------------------------------------------------------------


//---------------------------------------初始化前瞻缓冲区-----------------------------------------
	//声明前瞻缓冲区参数
	TLookAheadPrm LookAheadPrm;

	//各轴的最大加速度，单位脉冲/毫秒^2
	LookAheadPrm.dAccMax[0] = 0.2;
	LookAheadPrm.dAccMax[1] = 0.2;
	LookAheadPrm.dAccMax[2] = 0.2;

	//各轴的最大速度变化量（相当于启动速度）,单位脉冲/毫秒
	LookAheadPrm.dMaxStepSpeed[0] = 10;
	LookAheadPrm.dMaxStepSpeed[1] = 10;
	LookAheadPrm.dMaxStepSpeed[2] = 10;

	//各轴的脉冲当量(通常为1)
	LookAheadPrm.dScale[0] = 1;
	LookAheadPrm.dScale[1] = 1;
	LookAheadPrm.dScale[2] = 1;

	//各轴的最大速度(脉冲/毫秒)
	LookAheadPrm.dSpeedMax[0] = 100;
	LookAheadPrm.dSpeedMax[1] = 100;
	LookAheadPrm.dSpeedMax[2] = 100;
	LookAheadPrm.dSpeedMax[3] = 100;
	LookAheadPrm.dSpeedMax[4] = 100;

	//定义前瞻缓冲区长度
	LookAheadPrm.lookAheadNum = FROCAST_LEN;
	//定义前瞻缓冲区指针
	LookAheadPrm.pLookAheadBuf = LookAheadBuf;

//-------------------------------------------------------------------------------------------------------
	//初始化前瞻缓冲区
	//iRes = GA_InitLookAhead(1,0,&LookAheadPrm);
	GA_InitLookAheadSingle(1,0,LookAheadPrm.lookAheadNum,LookAheadPrm.dSpeedMax,LookAheadPrm.dAccMax,LookAheadPrm.dMaxStepSpeed,LookAheadPrm.dScale);
//-------------------------------------------------------------------------------------------------------

	if(0 == iRes)
	{
		AfxMessageBox("初始化前瞻缓冲区成功！");
	}
	else
	{
		AfxMessageBox("初始化前瞻缓冲区失败！");
		return;
	}
}


void CDemoVCDlg::OnBnClickedButton10()
{
	int iRes = 0;

	long lPosTemp[8];
	float dAcc[8];
	float dVel[8];
	

	memset(&lPosTemp,0,sizeof(lPosTemp));
	lPosTemp[3] = 50000;
	
	memset(dVel,0,sizeof(dVel));
	dVel[3] = 100;

	memset(dAcc,0,sizeof(dAcc));
	dAcc[3] = 0.001;

	//清空插补缓冲区原有数据
	//GA_CrdClear(1,0);

	//插入二维直线数据，X=10000,Y=20000,速度=50脉冲/ms，加速度=0.2脉冲/毫秒^2,终点速度为0，Fifo为0，用户自定义行号为1
	iRes = GA_LnXYZ(1,0,0,0,50,0.2,0,0,1);
	iRes = GA_LnXYZ(1,50000,0,0,50,0.2,0,0,2);

	//插补缓冲区里面插入非插补轴的运动数据，可以为阻塞，也可以为非阻塞
	iRes = GA_BufMoveAcc(1,0X0008,dAcc,0,0);
	iRes = GA_BufMoveVel(1,0X0008,dVel,0,0);
	iRes = GA_BufMove(1,0X0008,lPosTemp,0X0008,0,0);

	iRes = GA_LnXYZ(1,0,0,0,50,0.2,0,0,1);
	iRes = GA_LnXYZ(1,50000,0,0,50,0.2,0,0,2);
	iRes = GA_LnXYZ(1,0,0,0,50,0.2,0,0,1);
	iRes = GA_LnXYZ(1,50000,0,0,50,0.2,0,0,2);
	iRes = GA_LnXYZ(1,0,0,0,50,0.2,0,0,1);
	iRes = GA_LnXYZ(1,50000,0,0,50,0.2,0,0,2);
	iRes = GA_LnXYZ(1,0,0,0,50,0.2,0,0,1);
	iRes = GA_LnXYZ(1,50000,0,0,50,0.2,0,0,2);
	

	lPosTemp[3] = 0;
	iRes = GA_BufMove(1,0X0008,lPosTemp,0X0000,0,0);

	iRes = GA_LnXYZ(1,0,0,0,50,0.2,0,0,2);

	/*
	//插入三维直线数据，X=0,Y=0,Z=0速度=50脉冲/ms，加速度=0.2脉冲/毫秒^2,终点速度为0，Fifo为0，用户自定义行号为2
	iRes = GA_LnXYZ(1,0,0,0,50,0.2,0,0,2);

	//插入三维直线数据，X=10000,Y=20000,Z=2000,速度=50脉冲/ms，加速度=0.2脉冲/毫秒^2,终点速度为0，Fifo为0，用户自定义行号为3
	iRes += GA_LnXYZ(1,10000,20000,2000,50,0.2,0,0,3);

	//插入三维直线数据，X=0,Y=0,Z=0,速度=50脉冲/ms，加速度=0.2脉冲/毫秒^2,终点速度为0，Fifo为0，用户自定义行号为4
	iRes += GA_LnXYZ(1,0,0,0,50,0.2,0,0,4);

	//插入二维直线数据，X=10000,Y=20000,速度=50脉冲/ms，加速度=0.2脉冲/毫秒^2,终点速度为0，Fifo为0，用户自定义行号为5
	iRes = GA_LnXY(1,10000,20000,50,0.2,0,0,5);

	//插入IO输出指令（不会影响运动）
	iRes = GA_BufIO(1,MC_GPO,0,0X0001,0X0001,0,5);

	//插入二维直线数据，X=10000,Y=20000,速度=50脉冲/ms，加速度=0.2脉冲/毫秒^2,终点速度为0，Fifo为0，用户自定义行号为1
	iRes = GA_LnXY(1,50000,60000,50,0.2,0,0,6);

	//插入IO输出指令（不会影响运动）
	iRes = GA_BufIO(1,MC_GPO,0,0X0001,0X0000,0,6);

	//插入三维直线数据，X=0,Y=0,Z=0,速度=50脉冲/ms，加速度=0.2脉冲/毫秒^2,终点速度为0，Fifo为0，用户自定义行号为7
	iRes += GA_LnXYZ(1,0,0,0,50,0.2,0,0,7);
	*/
	//iRes += GA_ArcXYC(1,50000,50000,25000,0,0,50,0.2,0,0,8);

	//iRes = GA_BufDelay(1,2000,0,8);

	//iRes += GA_ArcXYC(1,50000,0,0,-25000,1,50,0.2,0,0,3);

	//iRes = GA_BufDelay(1,2000,0,8);

	//iRes = GA_LnXYZ(1,0,0,0,50,0.2,0,0,4);

	//插入最后一行标识符（系统会把前瞻缓冲区数据全部压入板卡）
	iRes += GA_CrdData(1,NULL,0);

	if(0 == iRes)
	{
		//AfxMessageBox("插入插补数据成功！");
	}
	else
	{
		//AfxMessageBox("插入插补数据失败！");
		return;
	}
}


void CDemoVCDlg::OnBnClickedButton11()
{
	int iRes = 0;
	iRes = GA_CrdStart(1,0);

	if(0 == iRes)
	{
		AfxMessageBox("启动坐标系1成功！");
	}
	else
	{
		AfxMessageBox("启动坐标系1失败！");
		return;
	}
}


void CDemoVCDlg::OnBnClickedButton12()
{
	GA_Stop(0XFF00,0XFF00);
}


void CDemoVCDlg::OnNMReleasedcaptureSliderOverRide(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iRes = 0;
	// TODO: 在此添加控件通知处理程序代码
	double dOverRide = 0;

	double dParm[16];
	memset(dParm,0,sizeof(dParm));

	CString strText;
	//获取滑块位置
	strText.Format("%d%%",m_SliderCrdVel.GetPos());
	GetDlgItem(IDC_STATIC_CRD_VEL_OVER_RIDE)->SetWindowText(strText);

	dOverRide = m_SliderCrdVel.GetPos()/100.00;
	iRes = GA_SetOverride(1,dOverRide);

	*pResult = 0;
}


void CDemoVCDlg::OnNMCustomdrawSliderOverRide(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


void CDemoVCDlg::OnBnClickedButtonSetCrdPrm2()
{
	int iRes = 0;

	UpdateData(true);

//---------------------------------------建立坐标系2------------------------------------------------
	//坐标系参数初始化
	TCrdPrm crdPrm;
	memset(&crdPrm,0,sizeof(crdPrm));

	//设置坐标系最小匀速时间为0
	crdPrm.evenTime = 0;

	//设置原点坐标值标志,0:默认当前规划位置为原点位置;1:用户指定原点位置
	crdPrm.setOriginFlag = 1;

	//设置坐标系原点
	
	crdPrm.originPos[0] = m_lOrginX2;
	crdPrm.originPos[1] = m_lOrginY2;
	crdPrm.originPos[2] = m_lOrginZ2;
	

	//设置坐标系维数为3
	crdPrm.dimension = 3;

	//设置坐标系规划对应轴通道，通道1对应轴1，通道2对应轴3，通道3对应轴5
	crdPrm.profile[0] = 4;
	crdPrm.profile[1] = 5;
	crdPrm.profile[2] = 6;


	//设置坐标系最大加速度为0.2脉冲/毫秒^2
	crdPrm.synAccMax = 0.2;
	//设置坐标系最大速度为100脉冲/毫秒
	crdPrm.synVelMax = 100;

	//设置坐标系参数，建立坐标系
	iRes = GA_SetCrdPrm(2,&crdPrm);

	if(0 == iRes)
	{
		AfxMessageBox("建立坐标系成功！");
	}
	else
	{
		AfxMessageBox("建立坐标系失败！确认是否重复建立坐标系或者尚未停止当前坐标系！");
		return;
	}
//------------------------------------------------------------------------------------------------


//---------------------------------------初始化前瞻缓冲区2-----------------------------------------
	//声明前瞻缓冲区参数
	TLookAheadPrm LookAheadPrm;

	//各轴的最大加速度，单位脉冲/毫秒^2
	LookAheadPrm.dAccMax[0] = 0.2;
	LookAheadPrm.dAccMax[1] = 0.2;
	LookAheadPrm.dAccMax[2] = 0.2;

	//各轴的最大速度变化量（相当于启动速度）,单位脉冲/毫秒
	LookAheadPrm.dMaxStepSpeed[0] = 10;
	LookAheadPrm.dMaxStepSpeed[1] = 10;
	LookAheadPrm.dMaxStepSpeed[2] = 10;

	//各轴的脉冲当量(通常为1)
	LookAheadPrm.dScale[0] = 1;
	LookAheadPrm.dScale[1] = 1;
	LookAheadPrm.dScale[2] = 1;

	//各轴的最大速度(脉冲/毫秒)
	LookAheadPrm.dSpeedMax[0] = 100;
	LookAheadPrm.dSpeedMax[1] = 100;
	LookAheadPrm.dSpeedMax[2] = 100;
	LookAheadPrm.dSpeedMax[3] = 100;
	LookAheadPrm.dSpeedMax[4] = 100;

	//定义前瞻缓冲区长度
	LookAheadPrm.lookAheadNum = FROCAST_LEN;
	//定义前瞻缓冲区指针
	LookAheadPrm.pLookAheadBuf = LookAheadBuf;

	//初始化前瞻缓冲区
	iRes = GA_InitLookAhead(2,0,&LookAheadPrm);

	if(0 == iRes)
	{
		AfxMessageBox("初始化前瞻缓冲区成功！");
	}
	else
	{
		AfxMessageBox("初始化前瞻缓冲区失败！");
		return;
	}
}


void CDemoVCDlg::OnBnClickedButton13()
{
	int iRes = 0;

	//清空插补缓冲区原有数据
	//GA_CrdClear(1,0);

	//插入二维直线数据，X=10000,Y=20000,速度=50脉冲/ms，加速度=0.2脉冲/毫秒^2,终点速度为0，Fifo为0，用户自定义行号为1
	iRes = GA_LnXYZ(2,0,50000,0,50,0.2,0,0,1);
	iRes = GA_LnXYZ(2,50000,0,0,50,0.2,0,0,2);
	iRes = GA_LnXYZ(2,0,0,50000,50,0.2,0,0,1);
	iRes = GA_LnXYZ(2,50000,0,0,50,0.2,0,0,2);
	iRes = GA_LnXYZ(2,0,0,0,50,0.2,0,0,1);
	iRes = GA_LnXYZ(2,50000,0,0,50,0.2,0,0,2);
	iRes = GA_LnXYZ(2,0,0,0,50,0.2,0,0,1);
	iRes = GA_LnXYZ(2,50000,0,0,50,0.2,0,0,2);
	iRes = GA_LnXYZ(2,0,0,0,50,0.2,0,0,1);
	iRes = GA_LnXYZ(2,50000,0,0,50,0.2,0,0,2);
	iRes = GA_LnXYZ(2,50000,0,0,50,0.2,0,0,2);


	//插入最后一行标识符（系统会把前瞻缓冲区数据全部压入板卡）
	iRes += GA_CrdData(2,NULL,0);

	if(0 == iRes)
	{
		//AfxMessageBox("插入插补数据成功！");
	}
	else
	{
		//AfxMessageBox("插入插补数据失败！");
		return;
	}
}


void CDemoVCDlg::OnBnClickedButton14()
{
	int iRes = 0;
	iRes = GA_CrdStart(2,0);

	if(0 == iRes)
	{
		AfxMessageBox("启动坐标系2成功！");
	}
	else
	{
		AfxMessageBox("启动坐标系2失败！");
		return;
	}
}


void CDemoVCDlg::OnBnClickedButton15()
{
	GA_Stop(0XFFFF,0XFFFF);
}


void CDemoVCDlg::OnNMReleasedcaptureSliderOverRide2(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	int iRes = 0;
	// TODO: 在此添加控件通知处理程序代码
	double dOverRide = 0;

	double dParm[16];
	memset(dParm,0,sizeof(dParm));

	CString strText;
	//获取滑块位置
	strText.Format("%d%%",m_SliderCrdVel2.GetPos());
	GetDlgItem(IDC_STATIC_CRD_VEL_OVER_RIDE2)->SetWindowText(strText);

	dOverRide = m_SliderCrdVel2.GetPos()/100.00;
	iRes = GA_SetOverride(1,dOverRide);

	*pResult = 0;
}
