@echo off

FOR /D %%i IN ("0*") DO (
    PUSHD "%%i"
    ECHO %%CD%%
	cmake -B cmake
	cmake --build cmake -j4
    POPD
)
