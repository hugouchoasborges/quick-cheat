# 0.2.0-alpha.4 — Markdown e imagens nos Steps

## Implementação e decisões
Texto dos Steps aceita Markdown básico: títulos, negrito, itálico, riscado, listas, checklists, citações, código e links. Popup renderiza com MD4C v0.6.0 fixado no commit 7fc1815a5eeba2af7d6120a76202bf59f3b6e6e4 e Rich Edit nativo, carregado sob demanda. Sem navegador embutido. Links http/https/mailto só abrem por clique; HTML não executa e imagens remotas não são baixadas. Tabelas e fórmulas não são renderizadas.

Attach image... no cadastro importa PNG/JPEG/BMP/GIF via WIC, copia como PNG gerenciado em attachments/ e insere referência Markdown na posição selecionada. Salvar o workflow persiste o texto. GIF usa primeiro frame. Originais não são alterados. Limites: entrada 10 MiB/24MP; normalização 1600×1600/4 MiB; até oito imagens renderizadas por Step; texto continua limitado a 4096 caracteres. Caminhos externos/traversal são recusados pelo leitor.

Schema permanece v3. O backup .qc contém referências, não pixels: copiar toda a pasta de dados para backup completo. Anexos sem referência são mantidos para preservar backups, inclusive após descartar uma edição. Renderização e imagens são liberadas ao ocultar/trocar aba; conteúdo inalterado usa cache e resize tem debounce pontual de 140 ms.

## Evidências
- Release x64 compilada com scripts/build.ps1 -SkipTests. Nenhum teste adicionado ou executado. Compilação inicial do parser upstream emite avisos MSVC; fontes vendorizadas mantidas sem alteração.
- Capturas DEMO isoladas em Dark/Light e cadastro inspecionadas. Corrigida posição do cursor após EM_INSERTIMAGE; confirmação mostra imagem na ordem correta, formatação, código e link, sem falso erro de imagem.
- Revisão independente identificou links adjacentes e entidades nos destinos. Corrigido mapeamento pelo caractere clicado e decodificação do destino antes da validação. Revisor confirmou ambas as correções por inspeção.
- Amostra de repouso antes de abrir UI: working set 14,68 MiB, private 2,80 MiB e CPU 0 ms em 1 segundo. Não constitui teto nem benchmark formal.
- Dados locais do ChatGPT: três fluxos existentes atualizados com Markdown e três diagramas originais ilustrativos. Nove atalhos preservados; demais perfis/fluxos mantidos. Backup anterior: shortcuts.qc.before-markdown-demo-15618687.bak. Nenhuma mensagem enviada ou conversa alterada.

## Aceitação manual pendente
1. No ChatGPT, abrir popup > Workflows e selecionar Steps dos três guias: conferir texto e imagens, rolagem e clareza nos temas Dark/Light/System.
2. Manage workflows: editar Markdown, anexar imagem, salvar, fechar/reabrir e reiniciar; referências e imagens devem permanecer.
3. Anexar formatos suportados, verificar erro amigável para arquivo grande/inválido, imagem ausente e limite de texto. Descartar edição não deve alterar o texto salvo.
4. Redimensionar/Expand/Restore, trocar escala/monitor e usar PIN. Imagens devem caber no leitor e texto permanecer acessível.
5. Clicar links, inclusive adjacentes e destinos com query string; apenas o destino correto deve abrir. Referências remotas/HTML não devem carregar conteúdo ativo.
6. Conferir atalhos existentes, seleção, Execute, clique duplo e contexto de app externo. Restaurar backup junto com attachments deve preservar os guias ilustrados.

## Entrega
Candidata 0.2.0-alpha.4 aguardando aceitação. Gitflow: feature/0.2.0-workflow-markdown-images, develop e release/0.2.0-alpha.4. main permanece no release publicado. Exemplos pessoais e anexos locais não integram o pacote.
