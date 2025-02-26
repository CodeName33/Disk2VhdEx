# Disk2VhdEx

[disk2vhd]([https://www.genome.gov/](https://learn.microsoft.com/en-us/sysinternals/downloads/disk2vhd)) utility from Sysinternals is very useful for creating backups and can be called from command line, 
but it has poor command line flags options.
My tool handles disk2vhd window and set all optiona that can be set in GUI mode.

# Advantages 
 - Can set all three checkboxes using command line (Vhdx, Virtual PC, Volume Shadow Copy)
 - Can select partitions using names or labels with wildcards (*)
 - Can run backup for selected parameters and close disk2vhd after it finishes

# Usage

Usage disk2vhdex.exe arg1=value arg2=value ...

Arguments:

  output=FILENAME - output file
  
  vhdx=(1/0) - use VhdX format
  
  shadowcopy=(1/0) - enable shadow volume copy
  
  virtualpc=(1/0) - virtual pc mode
  
  volume=VOLUME_NAME include volume with this name (can use * wildcard, case ignored). This option can be declared many times. volume=c:\ volume=d:\ ...
  label=VOLUME_LABEL - include volume with this labels (can use * wildcard, case ignored). This option can be declared many times. label=MY_DRIVE1 label=MY_DRIVE2 ...
  volumelabel=VOLUME_NAME=VOLUME_LABEL- include volume with this name and label (can use * wildcard, case ignored). This option can be declared many times. volumelabel=*volume*|[no label]
  showlist - show drives list (just 'showlist' without '=' and values)
  run - run vhd creating (just 'run' without '=' and values)
Example:
      disk2vhdex.exe output=D:\my_backup.vhdx vhdx=1 shadowcopy=1 virtualpc=0 volume=c:\ volume=e:\ label=*DATA run
      This options will create 'D:\my_backup.vhdx' and try to include volumes 'c:\' 'e:\' and volume(s) with label that ends with 'data' 
      disk2vhdex.exe showlist
      This will list all partitions with labels

disk2vhd64.exe from Sysinternals package must be in same folder with disk2vhdex or PATH to disk2vhd64 must be set.
