Imports System
Imports System.IO
Imports System.Collections

Module FindFile

    Function FindFile(ByVal FileName As String) As String
        Return FindFileAux(FileName, "C:\")
    End Function

    Private Function FindFileAux(ByVal FileName As String, ByVal Path As String) As String
        If File.Exists(Path + "\" + FileName) Then
            Return Path
        Else
            Dim directoryentry As String
            Dim foundpath As String
            For Each directoryentry In Directory.GetDirectories(Path)
                foundpath = FindFileAux(FileName, directoryentry)
                If (foundpath <> "") Then
                    Return foundpath
                End If
            Next
        End If
        Return ""
    End Function
End Module
