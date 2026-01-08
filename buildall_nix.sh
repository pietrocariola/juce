for d in 0??_*
do
    pushd $d
    echo "$d - $(pwd)"
    cmake -B cmake
    cmake --build cmake -j4
    popd >/dev/null 2>&1
done
