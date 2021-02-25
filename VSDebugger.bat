@echo off
@echo setting TCE Variables
SET TC_ROOT=D:\Apps\Siemens\TC11.6\IZTC5755\TC_ROOT
SET TC_DATA=D:\Apps\Siemens\TC11.6\IZTC5755\TC_DATA
call %TC_DATA%\tc_profilevars


@echo Launching Visual Studio
REM call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\WDExpress.exe"
call "D:\ITKWorkspace\TIACostDataImport\TIACostDataImport.sln"
REM call "D:\ITKWorkspace\TIPLMERPIntegrationSOA\TIPLMERPIntegration\TIPLMERPIntegration.sln"

