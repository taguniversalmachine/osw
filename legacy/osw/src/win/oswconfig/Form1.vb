Imports System.Text.RegularExpressions
Imports System
Imports System.IO
Imports System.Collections

Public Class Form1
    Inherits System.Windows.Forms.Form

#Region " Windows Form Designer generated code "

    Public Sub New()
        MyBase.New()

        'This call is required by the Windows Form Designer.
        InitializeComponent()

        'Add any initialization after the InitializeComponent() call

    End Sub

    'Form overrides dispose to clean up the component list.
    Protected Overloads Overrides Sub Dispose(ByVal disposing As Boolean)
        If disposing Then
            If Not (components Is Nothing) Then
                components.Dispose()
            End If
        End If
        MyBase.Dispose(disposing)
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    Friend WithEvents SourceDirBox As System.Windows.Forms.TextBox
    Friend WithEvents InstallDirBox As System.Windows.Forms.TextBox
    Friend WithEvents TclDirBox As System.Windows.Forms.TextBox
    Friend WithEvents Label1 As System.Windows.Forms.Label
    Friend WithEvents Label2 As System.Windows.Forms.Label
    Friend WithEvents Label3 As System.Windows.Forms.Label
    Friend WithEvents Button1 As System.Windows.Forms.Button
    Friend WithEvents Button2 As System.Windows.Forms.Button
    Friend WithEvents Button3 As System.Windows.Forms.Button
    Friend WithEvents OpenFileDialog1 As System.Windows.Forms.OpenFileDialog
    Friend WithEvents DebugBox As System.Windows.Forms.CheckBox
    Friend WithEvents Label4 As System.Windows.Forms.Label
    Friend WithEvents CompilerBox As System.Windows.Forms.ComboBox
    Friend WithEvents Button4 As System.Windows.Forms.Button
    Friend WithEvents DefaultButton As System.Windows.Forms.Button
    Friend WithEvents ExitButton As System.Windows.Forms.Button
    Friend WithEvents Label5 As System.Windows.Forms.Label
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Dim resources As System.Resources.ResourceManager = New System.Resources.ResourceManager(GetType(Form1))
        Me.SourceDirBox = New System.Windows.Forms.TextBox()
        Me.InstallDirBox = New System.Windows.Forms.TextBox()
        Me.TclDirBox = New System.Windows.Forms.TextBox()
        Me.Label1 = New System.Windows.Forms.Label()
        Me.Label2 = New System.Windows.Forms.Label()
        Me.Label3 = New System.Windows.Forms.Label()
        Me.Button1 = New System.Windows.Forms.Button()
        Me.Button2 = New System.Windows.Forms.Button()
        Me.Button3 = New System.Windows.Forms.Button()
        Me.DebugBox = New System.Windows.Forms.CheckBox()
        Me.OpenFileDialog1 = New System.Windows.Forms.OpenFileDialog()
        Me.CompilerBox = New System.Windows.Forms.ComboBox()
        Me.Label4 = New System.Windows.Forms.Label()
        Me.Button4 = New System.Windows.Forms.Button()
        Me.DefaultButton = New System.Windows.Forms.Button()
        Me.ExitButton = New System.Windows.Forms.Button()
        Me.Label5 = New System.Windows.Forms.Label()
        Me.SuspendLayout()
        '
        'SourceDirBox
        '
        Me.SourceDirBox.Location = New System.Drawing.Point(144, 40)
        Me.SourceDirBox.Name = "SourceDirBox"
        Me.SourceDirBox.Size = New System.Drawing.Size(336, 22)
        Me.SourceDirBox.TabIndex = 0
        Me.SourceDirBox.Text = ""
        '
        'InstallDirBox
        '
        Me.InstallDirBox.Location = New System.Drawing.Point(144, 80)
        Me.InstallDirBox.Name = "InstallDirBox"
        Me.InstallDirBox.Size = New System.Drawing.Size(336, 22)
        Me.InstallDirBox.TabIndex = 1
        Me.InstallDirBox.Text = ""
        '
        'TclDirBox
        '
        Me.TclDirBox.Location = New System.Drawing.Point(144, 120)
        Me.TclDirBox.Name = "TclDirBox"
        Me.TclDirBox.Size = New System.Drawing.Size(336, 22)
        Me.TclDirBox.TabIndex = 2
        Me.TclDirBox.Text = ""
        '
        'Label1
        '
        Me.Label1.Font = New System.Drawing.Font("Arial", 10.2!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label1.Location = New System.Drawing.Point(24, 40)
        Me.Label1.Name = "Label1"
        Me.Label1.TabIndex = 3
        Me.Label1.Text = "Source Dir"
        '
        'Label2
        '
        Me.Label2.Font = New System.Drawing.Font("Arial", 10.2!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label2.Location = New System.Drawing.Point(24, 80)
        Me.Label2.Name = "Label2"
        Me.Label2.TabIndex = 4
        Me.Label2.Text = "Install Dir"
        '
        'Label3
        '
        Me.Label3.Font = New System.Drawing.Font("Arial", 10.2!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label3.Location = New System.Drawing.Point(24, 120)
        Me.Label3.Name = "Label3"
        Me.Label3.TabIndex = 5
        Me.Label3.Text = "Tcl Dir"
        '
        'Button1
        '
        Me.Button1.Image = CType(resources.GetObject("Button1.Image"), System.Drawing.Bitmap)
        Me.Button1.Location = New System.Drawing.Point(480, 40)
        Me.Button1.Name = "Button1"
        Me.Button1.Size = New System.Drawing.Size(24, 24)
        Me.Button1.TabIndex = 6
        '
        'Button2
        '
        Me.Button2.Image = CType(resources.GetObject("Button2.Image"), System.Drawing.Bitmap)
        Me.Button2.Location = New System.Drawing.Point(480, 80)
        Me.Button2.Name = "Button2"
        Me.Button2.Size = New System.Drawing.Size(24, 24)
        Me.Button2.TabIndex = 7
        '
        'Button3
        '
        Me.Button3.Image = CType(resources.GetObject("Button3.Image"), System.Drawing.Bitmap)
        Me.Button3.Location = New System.Drawing.Point(480, 120)
        Me.Button3.Name = "Button3"
        Me.Button3.Size = New System.Drawing.Size(24, 24)
        Me.Button3.TabIndex = 8
        '
        'DebugBox
        '
        Me.DebugBox.Font = New System.Drawing.Font("Arial", 10.2!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.DebugBox.Location = New System.Drawing.Point(384, 160)
        Me.DebugBox.Name = "DebugBox"
        Me.DebugBox.TabIndex = 9
        Me.DebugBox.Text = "Debug"
        '
        'CompilerBox
        '
        Me.CompilerBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList
        Me.CompilerBox.Items.AddRange(New Object() {"Visual C++ .NET"})
        Me.CompilerBox.Location = New System.Drawing.Point(144, 160)
        Me.CompilerBox.Name = "CompilerBox"
        Me.CompilerBox.Size = New System.Drawing.Size(208, 24)
        Me.CompilerBox.TabIndex = 10
        '
        'Label4
        '
        Me.Label4.Font = New System.Drawing.Font("Arial", 10.2!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label4.Location = New System.Drawing.Point(24, 160)
        Me.Label4.Name = "Label4"
        Me.Label4.TabIndex = 11
        Me.Label4.Text = "Compiler"
        '
        'Button4
        '
        Me.Button4.Font = New System.Drawing.Font("Arial", 10.2!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Button4.Location = New System.Drawing.Point(80, 224)
        Me.Button4.Name = "Button4"
        Me.Button4.Size = New System.Drawing.Size(112, 40)
        Me.Button4.TabIndex = 12
        Me.Button4.Text = "Generate"
        '
        'DefaultButton
        '
        Me.DefaultButton.Font = New System.Drawing.Font("Arial", 10.2!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.DefaultButton.Location = New System.Drawing.Point(216, 224)
        Me.DefaultButton.Name = "DefaultButton"
        Me.DefaultButton.Size = New System.Drawing.Size(112, 40)
        Me.DefaultButton.TabIndex = 13
        Me.DefaultButton.Text = "Default"
        '
        'ExitButton
        '
        Me.ExitButton.DialogResult = System.Windows.Forms.DialogResult.Cancel
        Me.ExitButton.Font = New System.Drawing.Font("Arial", 10.2!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.ExitButton.Location = New System.Drawing.Point(352, 224)
        Me.ExitButton.Name = "ExitButton"
        Me.ExitButton.Size = New System.Drawing.Size(112, 40)
        Me.ExitButton.TabIndex = 14
        Me.ExitButton.Text = "Exit"
        '
        'Label5
        '
        Me.Label5.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.Label5.FlatStyle = System.Windows.Forms.FlatStyle.Flat
        Me.Label5.Location = New System.Drawing.Point(8, 200)
        Me.Label5.Name = "Label5"
        Me.Label5.Size = New System.Drawing.Size(528, 8)
        Me.Label5.TabIndex = 15
        '
        'Form1
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(6, 15)
        Me.CancelButton = Me.ExitButton
        Me.ClientSize = New System.Drawing.Size(544, 304)
        Me.Controls.AddRange(New System.Windows.Forms.Control() {Me.Label5, Me.ExitButton, Me.DefaultButton, Me.Button4, Me.Label4, Me.CompilerBox, Me.DebugBox, Me.Button3, Me.Button2, Me.Button1, Me.Label3, Me.Label2, Me.Label1, Me.TclDirBox, Me.InstallDirBox, Me.SourceDirBox})
        Me.Name = "Form1"
        Me.Text = "OSW Windows Build Configurator"
        Me.ResumeLayout(False)

    End Sub

#End Region

    Dim WinSrcDir As String

    Private Sub Button1_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button1.Click

    End Sub

    Private Sub ExitButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ExitButton.Click
        End
    End Sub

    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        WinSrcDir = CurDir()
        While Not File.Exists(WinSrcDir + "\Makefile.vc7")
            WinSrcDir = System.IO.Directory.GetParent(WinSrcDir).FullName()
        End While
        GetDefaults()
    End Sub

    Private Sub GetDefaults()
        DefaultSourceDir()
        DefaultInstallDir()
        DefaultTclDir()
        CompilerBox.Text = CompilerBox.Items(0)
    End Sub

    Private Sub DefaultSourceDir()
        Dim FindDir As String
        FindDir = CurDir()
        While Dir(FindDir + "\src", FileAttribute.Directory) <> "src"
            FindDir = System.IO.Directory.GetParent(FindDir).FullName
        End While
        SourceDirBox.Text = FindDir
    End Sub

    Private Sub DefaultInstallDir()
        InstallDirBox.Text = "C:\osw"
    End Sub

    Private Sub DefaultTclDir()
        Dim r As Regex
        r = New Regex("[cC]:\\[^:]+Tcl")
        Dim m As Match
        m = r.Match(System.Environment.GetEnvironmentVariable("PATH"))
        If m.Success Then
            TclDirBox.Text = m.ToString()
        Else
            r = New Regex("\/c\/([^:]+Tcl)")
            m = r.Match(System.Environment.GetEnvironmentVariable("PATH"))
            If m.Success Then
                TclDirBox.Text = m.Groups(0).ToString
            Else
                TclDirBox.Text = System.IO.Directory.GetParent(FindFile.FindFile("tcl82.lib")).FullName
            End If
        End If
    End Sub

    Private Sub DefaultButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles DefaultButton.Click
        GetDefaults()
    End Sub

    Private Sub Button4_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button4.Click
        If CompilerBox.Text = "Visual C++ .NET" Then
            Dim SystemMak As StreamWriter
            File.Copy(WinSrcDir + "\Makefile.vc7", WinSrcDir + "\Makefile", True)
            SystemMak = File.CreateText(WinSrcDir + "\System.mak")
            SystemMak.WriteLine("#System.mak for OSW Windows Source Build")
            SystemMak.WriteLine("#Created " + DateTime.Today)
            SystemMak.WriteLine("")
            SystemMak.WriteLine("OSW_SOURCE_DIR=" + SourceDirBox.Text)
            SystemMak.WriteLine("INSTALL_DIR=" + InstallDirBox.Text)
            SystemMak.WriteLine("TCL_DIR=" + TclDirBox.Text)
            If DebugBox.Checked Then
                SystemMak.WriteLine("USEDEBUG=1")
            End If
            SystemMak.Close()
            MsgBox("Makefile generated")
        End If
    End Sub
End Class
