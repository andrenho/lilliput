" files format
set fileformat=unix

set ts=4
set sts=4
set sw=4
set expandtab
set foldmethod=marker

" make program
set makeprg=clear\ \&\&\ make
nnoremap <Leader>T :!clear && make test<cr>
nnoremap <Leader>C :make! clean<cr>
nnoremap <Leader>r :!clear && ./lilliput<cr>
nnoremap <Leader>G :!git commit -a && git push<CR>

" open all files
e src/main.c
args Makefile
args src/*.h
args src/*.c
args lib/luisavm.h
args lib/*.c
args test/test.lua
args bindings/lua/luisavm.c
args .vimrc
b 1
map <Leader>x   :b src/main.c<CR>
map <Leader>h   :b lib/luisavm.h<CR>
map <Leader>m   :b lib/computer.c<CR>
map <Leader>c   :b lib/cpu.c<CR>
map <Leader>t   :b test/test.lua<CR>
map <Leader>l   :b bindings/lua/luisavm.c<CR>
map <Leader>M   :b Makefile<CR>
map <Leader>V   :b .vimrc<CR>

" swap between cc and hh
function! SwapBuffers()
  if expand("%:e") == "c"
    exe "b" fnameescape(expand("%:r").".h")
  elseif expand("%:e") == "h"
    exe "b" fnameescape(expand("%:r").".c")
  endif
endfunction
map <Leader>.  :call SwapBuffers()<CR>
