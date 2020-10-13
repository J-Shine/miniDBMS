syntax on
filetype plugin indent on

set expandtab
set tabstop=4
set shiftwidth=4

set nowrap
set clipboard=unnamedplus
set ignorecase
set incsearch

call plug#begin('~/.vim/plugged')
Plug 'vim-scripts/xoria256.vim'
Plug 'vim-scripts/peaksea'
call plug#end()

colorscheme peaksea 

autocmd FileType * setlocal formatoptions-=c formatoptions-=r formatoptions-=o
