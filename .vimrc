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
nnoremap <Leader>y :!clear && ./las -t<cr>
nnoremap <Leader>C :make! clean<cr>
nnoremap <Leader>r :!clear && make debug && ./luisavm -M map test.bin<cr>
nnoremap <Leader>G :!git commit -a && git push<CR>

" open all files
e src/main.cc
args Makefile
args src/*.hh
args src/*.cc
args lib/*.hh
args lib/*.cc
args .vimrc
b 1
map <Leader>x   :b src/main.cc<CR>
map <Leader>h   :b lib/luisavm.hh<CR>
map <Leader>m   :b lib/luisavm.cc<CR>
map <Leader>c   :b lib/cpu.cc<CR>
map <Leader>v   :b lib/video.cc<CR>
map <Leader>d   :b lib/debugger.cc<CR>
map <Leader>t   :b src/test.cc<CR>
map <Leader>M   :b Makefile<CR>
map <Leader>V   :b .vimrc<CR>

" swap between cc and hh
function! SwapBuffers()
  if expand("%:e") == "cc"
    exe "b" fnameescape(expand("%:r").".hh")
  elseif expand("%:e") == "hh"
    exe "b" fnameescape(expand("%:r").".cc")
  endif
endfunction
map <Leader>.  :call SwapBuffers()<CR>
