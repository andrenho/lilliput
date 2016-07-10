" files format
set fileformat=unix

set ts=4
set sts=4
set sw=4
set expandtab
set foldmethod=marker

" make program
set makeprg=clear\ \&\&\ make
nnoremap <Leader>b :make! debug<cr>
nnoremap <Leader>B :make! debug FORCE_COLOR=1<cr>
nnoremap <Leader>T :!clear && make debug && ./lilliput -T<cr>
nnoremap <Leader>L :make! debug CXX=clang++<cr>
nnoremap <Leader>C :make! clean<cr>
nnoremap <Leader>r :!clear && ./lilliput<cr>
nnoremap <Leader>G :!git commit -a && git push<CR>

" open all files
e src/main.c
args Makefile
args src/*.h
args src/*.c
args test/tests.py
args .vimrc
b 1
map <Leader>x   :b src/main.c<CR>
map <Leader>c   :b src/cpu.c<CR>
map <Leader>m   :b src/memory.c<CR>
map <Leader>v   :b src/video.c<CR>
map <Leader>d   :b src/debugger.c<CR>
map <Leader>t   :b src/tests.c<CR>
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
