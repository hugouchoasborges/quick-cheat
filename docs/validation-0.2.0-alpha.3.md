# 0.2.0-alpha.3 — PIN e blocos responsivos

## Comportamento solicitado e implementado
PIN no canto superior direito, desligado ao iniciar o processo. Desligado: perder foco oculta a popup, mantendo o app na bandeja. Ligado: a popup permanece aberta ao alternar aplicativos, executar atalhos ou acionar a hotkey; Escape com foco na popup ou Close a oculta. Close aparece ao lado de PIN apenas quando ativo. Estado mantido durante a sessão, sem mudança no schema v3.
Cadastro é uma janela proprietária acima da popup fixada, mantendo acesso à biblioteca e fluxos. Alternar apps externos continua atualizando o contexto; a própria UI preserva o último contexto válido.

## Blocos de atalhos
Native Win32 ListView em modo Tile, com desenho GDI e seleção/navegação nativas. Cada bloco reúne categoria, descrição e teclas. Colunas são recalculadas pela largura/DPI, sem espaços vazios de grupos separados. Descrições longas ou combinações que não cabem no bloco podem ser lidas na área de detalhes.
Clique simples seleciona; clique duplo, Execute e Enter na busca executam. Clique duplo valida janela/processo/item capturados no primeiro clique e é cancelado após atualização do contexto. Setas na busca percorrem todos os resultados sequencialmente; navegação dentro da grade usa o controle nativo.
Referência da API: https://learn.microsoft.com/en-us/windows/win32/controls/use-tile-views

## Revisão e validação
Build Release x64 compilada com scripts/build.ps1 -SkipTests; nenhum teste adicionado ou executado. Helper de integração existente adaptado para ListView e PIN no cenário de troca de contexto, sem execução. Revisão identificou salto de colunas que tornava resultados inacessíveis pelas setas da busca; corrigido para navegação sequencial e confirmado por inspeção.
Capturas DEMO isoladas em Dark/Light/PIN/Workflows inspecionadas. Uma captura inicial expôs sobreposição dos blocos na primeira abertura: corrigido dimensionando antes de inserir itens e invalidando posições nativas em mudanças de tamanho/DPI. Confirmação visual direcionada comprovou a grade inicial sem sobreposição. Sem alegação de validação completa de monitores/DPI ou das transições de foco no ambiente do usuário.
Amostra isolada em repouso: working set 14.64 MiB, private 2.80 MiB, CPU 0 ms em 1 segundo; não é teto nem benchmark formal.

## Aceitação manual pendente
1. Abrir com PIN desligado, clicar fora/Alt+Tab: popup deve sumir e processo continuar na bandeja; reabrir pela hotkey.
2. Ligar PIN: Close aparece ao lado. Alternar apps e clicar fora: popup continua aberta e contexto segue o app externo correto.
3. Executar atalho com PIN ligado: popup continua aberta; desligado: oculta ao retornar foco ao destino.
4. Com PIN ligado, usar hotkey, Manage shortcuts e Manage workflows; cadastro deve ser utilizável acima da popup.
5. Escape com foco na popup e Close devem ocultá-la. PIN fica na sessão; ao reiniciar processo, volta desligado.
6. Conferir grade na primeira abertura, busca, setas, clique simples, duplo clique e Execute; nenhum item deve ficar inacessível ou executar ao apenas selecionar.
7. Redimensionar/Expand/Restore e trocar escala/monitor; blocos não podem sobrepor-se. Conferir texto longo e combinação extensa no leitor.

## Dados e entrega
Dados locais e schema v3 preservados. Candidata aguarda aceitação/publicação; main permanece no release público alpha.4. Gitflow: feature/0.2.0-pin-and-shortcut-tiles, develop e release/0.2.0-alpha.3.