VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form ConTree 
   Caption         =   "1プロパティ前"
   BeginProperty Images {2C247F25-8591-11D1-B16A-00C0F0283628} 
      Caption   =   "2プロパティ中"
      BeginProperty ListImage1 {2C247F27-8591-11D1-B16A-00C0F0283628} 
         Picture         =   "CustomerDlg.frx":0035
         Caption         =   "3込みプロパティ中"
         Key             =   ""
      EndProperty
      Caption   =   "4込みプロパティ後"
   EndProperty
   Caption         =   "5プロパティ後"
   Begin VB.CommandButton CreatButton 
      Caption         =   "6込み中"
   End
   
   Caption　=　"7中"
   
   Begin MSComctlLib.ImageList ImageList1 
      Name = "8単コンメト前"　'単コンメト
   End
   
   Begin MSComctlLib.ImageList ImageList1 
      '単コメント
      Name = "9フォーム単コメント後"
      Caption(0) = "18小括号タイトル0"
      Caption(1) = "19小括号タイトル1"
   End
   Name = "10二Eng中"
End

Public strName AS String = "11外部"

Public Sub GetConKind(ConID As String) As String
   Dim Msg AS String
   Msg = "12内部"
   Msg = "13" '13単コメントエラー
   Msg = "14コード単コメント後"
   '多列コメント上部　_
多列コメント下部   
   Msg = "15多列コメント後"
   Msg = "16""Escape序列"
   Msg = "17定数"
End Sub
