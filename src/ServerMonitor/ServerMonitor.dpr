program ServerMonitor;

uses
  Forms,
  Main in 'Main.pas' {Form1},
  Unit2 in 'Unit2.pas' {Form2};

{$R *.res}

begin
    Application.Initialize;
    Application.CreateForm(TForm1, Form1);
  Application.Run;
end.

