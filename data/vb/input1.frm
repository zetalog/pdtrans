VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form ConTree 
   Caption         =   "1�v���p�e�B�O"
   BeginProperty Images {2C247F25-8591-11D1-B16A-00C0F0283628} 
      Caption   =   "2�v���p�e�B��"
      BeginProperty ListImage1 {2C247F27-8591-11D1-B16A-00C0F0283628} 
         Picture         =   "CustomerDlg.frx":0035
         Caption         =   "3���݃v���p�e�B��"
         Key             =   ""
      EndProperty
      Caption   =   "4���݃v���p�e�B��"
   EndProperty
   Caption         =   "5�v���p�e�B��"
   Begin VB.CommandButton CreatButton 
      Caption         =   "6���ݒ�"
   End
   
   Caption�@=�@"7��"
   
   Begin MSComctlLib.ImageList ImageList1 
      Name = "8�P�R�����g�O"�@'�P�R�����g
   End
   
   Begin MSComctlLib.ImageList ImageList1 
      '�P�R�����g
      Name = "9�t�H�[���P�R�����g��"
      Caption(0) = "18�������^�C�g��0"
      Caption(1) = "19�������^�C�g��1"
   End
   Name = "10��Eng��"
End

Public strName AS String = "11�O��"

Public Sub GetConKind(ConID As String) As String
   Dim Msg AS String
   Msg = "12����"
   Msg = "13" '13�P�R�����g�G���[
   Msg = "14�R�[�h�P�R�����g��"
   '����R�����g�㕔�@_
����R�����g����   
   Msg = "15����R�����g��"
   Msg = "16""Escape����"
   Msg = "17�萔"
End Sub
