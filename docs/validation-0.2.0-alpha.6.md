# 0.2.0-alpha.6 — Categorias expansíveis nos atalhos

## Alterações e decisões
Atalhos agrupados em seções nativas do ListView: seta, nome da categoria e divisor horizontal, com blocos abaixo. Clique em qualquer parte do cabeçalho alterna o estado; Left/Right/Space funcionam no cabeçalho focado. Categoria deixa de repetir dentro do bloco; altura reduzida para 92px. Categorias que diferem apenas por caixa compartilham seção; sem categoria integra General.

Estado por executável/categoria salvo no arquivo ui-state.ini junto de shortcuts.qc, respeitando --data-dir. Chaves hexadecimais de texto normalizado evitam caracteres especiais no INI. Categorias novas/renomeadas começam expandidas; preferências antigas são mantidas. Busca expande temporariamente os grupos de resultados e não grava esses estados. Gravação ocorre nas interações e antes de troca de contexto, refresh, ocultação/saída, sem polling. Falha de gravação produz aviso. Backups completos incluem ui-state.ini.

Seleção e execução validam visibilidade; recolher uma categoria limpa a seleção oculta e invalida o clique pendente. Setas da busca pulam itens recolhidos. Todos os grupos recolhidos continuam mostrando seus cabeçalhos. Schema v3, dados pessoais e fluxos não alterados.

## Evidências
- Release x64 compilada com scripts/build.ps1 -SkipTests; nenhum teste adicionado ou executado.
- Capturas DEMO Dark/Light: primeira inspeção identificou cabeçalhos nativos sem as cores do tema; ajustada pintura final dos cabeçalhos. Confirmação mostra divisores, nomes, setas e blocos compactos sem sobreposição.
- Inspeção do caminho de persistência, busca, seleção/execução e troca de contexto. Arquivo de preferências usa a chave do perfil que originou os grupos, evitando gravar estado no app seguinte.
- git diff --check sem erros. Sem alegação de teste automatizado de persistência/reinício ou validação completa de DPI/High Contrast/acessibilidade.

## Aceitação manual pendente
1. No ChatGPT, recolher algumas categorias e manter outras abertas; fechar/reabrir popup e reiniciar o app. Cada estado deve reaparecer.
2. Em outro aplicativo, usar categorias de mesmo nome com estados diferentes; alternar apps e conferir independência.
3. Recolher todas: cabeçalhos continuam acessíveis, Execute fica desabilitado e Enter não executa item oculto.
4. Buscar atalho de categoria recolhida: deve aparecer; limpar busca restaura o estado anterior. Setas percorrem apenas itens de grupos abertos.
5. Mouse no cabeçalho não executa atalhos; clique simples/duplo e Execute nos blocos mantêm os comportamentos anteriores. Conferir Left/Right/Space com cabeçalho focado.
6. Redimensionar, trocar temas/DPI/monitor, navegar com teclado e conferir nomes longos, categorias vazias e variantes de caixa.

## Entrega
Gitflow: feature/0.2.0-collapsible-shortcut-categories → develop → release/0.2.0-alpha.6. Candidata aguardando aceitação; main permanece no release publicado.

## APIs de referência
- https://learn.microsoft.com/en-us/windows/win32/api/commctrl/ns-commctrl-lvgroup
- https://learn.microsoft.com/en-us/windows/win32/controls/lvm-getgrouprect
