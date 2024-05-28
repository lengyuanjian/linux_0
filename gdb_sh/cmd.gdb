set env PAGET cat 
set pagination off
b main
commands
set $i=0
  while $i < 5
    printf "i = %d\n", $i
    set $i=$i+1
  end
  c
end
r
