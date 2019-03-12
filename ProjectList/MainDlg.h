// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "TreeCtrl.h"
#include "json/json.h"
#include "util/Path.h"
#include "util/StringEx.h"
#include "util/File.h"
#include "util/StringConverter.h"
#include <vector>

struct ONE_PROJECT_INFO
{
	std::string id;
	std::string name;
	std::string brief;
	std::string detail;
	std::string path;
	std::string repo;
};

struct PROJECT_CLASS_SP
{
	std::string className;
	std::string classBrief;
	std::vector<ONE_PROJECT_INFO> projects;
};




class CMainDlg : public CDialogImpl<CMainDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		NOTIFY_HANDLER_EX(IDC_TREE1, TVN_SELCHANGED, OnTreeSelChanged)
		NOTIFY_HANDLER_EX(IDC_TREE1, NM_DBLCLK, OnTreeDblClick)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	void OpenDirect(const std::string& dir)
	{
		ShellExecute(NULL, _T("open"), _T("explorer.exe"), String(dir).toStdWString().c_str(), NULL, SW_SHOWNORMAL);
	}

	LRESULT OnTreeDblClick(LPNMHDR lpNMHDR)
	{
		LPNMTREEVIEW lpTreeView = reinterpret_cast<LPNMTREEVIEW>(lpNMHDR);
		HTREEITEM hi = tree.GetSelectedItem();
		CString str;
		if (tree.GetItemText(hi, str))
		{
			if (str.GetLength() > 2 && str[0] == _T('['))
			{
				int posNext = str.Find(_T("]"), 1);
				if (posNext == -1)
				{
					goto OnTreeDblClickEnd;
				}
				else
				{
					CString ATLToFindId = (LPCTSTR)str.Mid(1, posNext - 1);
					if (ATLToFindId.IsEmpty())
					{
						goto OnTreeDblClickEnd;
					}
					std::string ToFindId = String::fromStdWString((LPCTSTR)ATLToFindId);
					for (auto i : m_projectsClassSpecified)
					{
						for (auto j : i.projects)
						{
							if (j.id == ToFindId)
							{
								std::string path = j.path;
								OpenDirect(path);
								goto OnTreeDblClickEnd;
							}
						}
					}
				}
			}
		}
	OnTreeDblClickEnd:
		SetMsgHandled(FALSE);
		return TRUE;
	}
	LRESULT OnTreeSelChanged(LPNMHDR lpNMHDR)
	{
		LPNMTREEVIEW lpTreeView = reinterpret_cast<LPNMTREEVIEW>(lpNMHDR);
		CString str;
		tree.GetItemText(lpTreeView->itemNew.hItem, str);
		if (str.GetLength() > 2 && str[0] == _T('['))
		{
			int posNext = str.Find(_T("]"), 1);
			if (posNext == -1)
			{
				goto OnTreeSelChangedEnd;
			}
			else
			{
				CString ATLToFindId = (LPCTSTR)str.Mid(1, posNext - 1);
				if (ATLToFindId.IsEmpty())
				{
					goto OnTreeSelChangedEnd;
				}
				std::string ToFindId = String::fromStdWString((LPCTSTR)ATLToFindId);
				for (auto i : m_projectsClassSpecified)
				{
					for (auto j : i.projects)
					{
						if (j.id == ToFindId)
						{
							std::string detail = j.detail.empty() ? j.brief : j.detail;
							detail += "\r\nlocal path: ";
							detail += j.path;
							detail += "\r\nrepo path: ";
							detail += j.repo;
							SetDlgItemText(IDC_EDIT1, StringConverter::UTF8ToUnicode(detail).c_str());
							goto OnTreeSelChangedEnd;
						}
					}
				}
			}
		}
	OnTreeSelChangedEnd:
		SetMsgHandled(FALSE);
		return TRUE;
	}

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
		SetIcon(hIconSmall, FALSE);
		
		tree.SubclassWindow(GetDlgItem(IDC_TREE1));

		if (!m_ilItemImages.CreateFromImage(IDB_EXAMPLE, 16, 0, RGB(255, 0, 255), IMAGE_BITMAP, LR_CREATEDIBSECTION))
			return FALSE;

		tree.SetImageList(m_ilItemImages);

		std::string jsonFile = Path::getApplicationDirPath();
		jsonFile += "projs.json";
		Json::Value jvRoot;
		Json::Reader jr;
		FileReader fr(jsonFile);
		std::string content;
		if (!fr.open())
		{
			MessageBox(_T("∂¡»°≈‰÷√ ß∞‹¡À0"));
			return FALSE;
		}
		content = fr.read();
		fr.close();
		if (!jr.parse(content.c_str(), jvRoot))
		{
			MessageBox(_T("∂¡»°≈‰÷√ ß∞‹¡À1"));
			return FALSE;
		}
		std::vector<std::string> classes = jvRoot.getMemberNames();
		if (classes.empty())
		{
			MessageBox(_T("∂¡»°≈‰÷√ ß∞‹¡À2"));
			return FALSE;
		}
		for (auto i : classes)
		{
			Json::Value jv = jvRoot[i];
			if (!jv.isObject())
			{
				return false;
			}
			else
			{
				PROJECT_CLASS_SP pcs;
				pcs.className = i;
				pcs.classBrief = jv["class_brif"].asString();
				CString str = StringConverter::UTF8ToUnicode(pcs.classBrief).c_str();// --
				HTREEITEM hParent = tree.InsertItem(str+_T("    "), 0, 1, TVI_ROOT, TVI_LAST);// --
				Json::Value jvArr = jv["projects"];
				for (int i = 0; i != jvArr.size(); i++)
				{
					ONE_PROJECT_INFO opi;
					Json::Value jvp = jvArr[i];
					opi.id = jvp["id"].asString();
					opi.name = jvp["name"].asString();
					opi.brief = jvp["brief"].asString();
					opi.detail = jvp["detail"].asString();
					opi.path = jvp["path"].asString();
					opi.repo = jvp["repo"].asString();
					pcs.projects.push_back(opi);
					std::string itemText = std::string("[") + opi.id + "]" + "[" + opi.name + "]" + opi.brief;// --
					CString strItem = StringConverter::UTF8ToUnicode(itemText).c_str();// --
					tree.InsertItem(strItem+_T("    "), 0, 1, hParent, TVI_LAST);// --
				}
				tree.Expand(hParent, TVE_EXPAND); // --
				m_projectsClassSpecified.push_back(pcs);
			}
		}
		tree.SetMultipleSelect(TRUE);
		tree.SetFocus();
		return TRUE;
	}

	LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CSimpleDialog<IDD_ABOUTBOX, FALSE> dlg;
		dlg.DoModal();
		return 0;
	}

	LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add validation code 
		EndDialog(wID);
		return 0;
	}

	LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
public:
	CTreeCtrl tree;
	CImageList m_ilItemImages;
	std::vector<PROJECT_CLASS_SP> m_projectsClassSpecified;
};
