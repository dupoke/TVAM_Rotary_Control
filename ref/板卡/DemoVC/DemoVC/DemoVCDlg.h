
// DemoVCDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CDemoVCDlg 对话框
class CDemoVCDlg : public CDialogEx
{
// 构造
public:
	CDemoVCDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_DEMOVC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton4();
	CComboBox m_ComboBoxAxis;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheckNoEncode();
	afx_msg void OnBnClickedButtonClrSts();
	afx_msg void OnBnClickedButtonAxisOn();
	afx_msg void OnBnClickedButton6();
	long m_lTargetPos;
	afx_msg void OnCbnSelchangeCombo1();
	CComboBox m_ComboBoxCardSel;
	afx_msg void OnBnClickedRadioInput1();
	afx_msg void OnBnClickedButtonOutPut1();
	afx_msg void OnBnClickedButtonOutPut2();
	afx_msg void OnBnClickedButtonOutPut3();
	afx_msg void OnBnClickedButtonOutPut4();
	afx_msg void OnBnClickedButtonOutPut5();
	afx_msg void OnBnClickedButtonOutPut6();
	afx_msg void OnBnClickedButtonOutPut7();
	afx_msg void OnBnClickedButtonOutPut8();
	afx_msg void OnBnClickedButtonOutPut9();
	afx_msg void OnBnClickedButtonOutPut10();
	afx_msg void OnBnClickedButtonOutPut11();
	afx_msg void OnBnClickedButtonOutPut12();
	afx_msg void OnBnClickedButtonOutPut13();
	afx_msg void OnBnClickedButtonOutPut14();
	afx_msg void OnBnClickedButtonOutPut15();
	afx_msg void OnBnClickedButtonOutPut16();
	afx_msg void OnBnClickedCheckPosLimEnable();
	afx_msg void OnBnClickedCheckNegLimEnable();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton5();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonJogPos();
	afx_msg void OnBnClickedButtonJogNeg();
	afx_msg void OnBnClickedButtonSetCrdPrm();
	afx_msg void OnBnClickedButton10();
	afx_msg void OnBnClickedButton11();
	afx_msg void OnBnClickedButton12();
	CSliderCtrl m_SliderCrdVel;
	CSliderCtrl m_SliderCrdVel2;
	afx_msg void OnNMReleasedcaptureSliderOverRide(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderOverRide(NMHDR *pNMHDR, LRESULT *pResult);
	long m_lOrginX;
	long m_lOrginY;
	long m_lOrginZ;
	long m_lOrginX2;
	long m_lOrginY2;
	long m_lOrginZ2;
	afx_msg void OnBnClickedButtonSetCrdPrm2();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton14();
	afx_msg void OnBnClickedButton15();
	afx_msg void OnNMReleasedcaptureSliderOverRide2(NMHDR *pNMHDR, LRESULT *pResult);
};
