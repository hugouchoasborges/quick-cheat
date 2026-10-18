# 0.2.0-alpha.7 — Settings em três abas

## Implementação
General concentra Appearance (System/Light/Dark), activation shortcut com Apply e acesso à pasta de dados. Shortcuts tem seletor de aplicativo, lista e formulário de descrição/categoria/teclas. Workflows compartilha o aplicativo selecionado e incorpora o editor numa janela filha: fluxos, steps e texto Markdown em três colunas, com anexos e Save workflow. Não abre janela modal separada. Menu da bandeja abre Settings/General; botões da popup entram na aba correspondente.

Layouts e visibilidade ficam em settings.cpp/settings.hpp, reaproveitando cadastro, persistência, gravação de atalhos e editor de fluxos. UI continua Win32/C++ sem dependências novas, com os temas existentes. Janela padrão 1120×860; mínimo 980×780 logical pixels. Aplicativo escolhido é mantido entre abas na sessão.

Mudanças pendentes em atalho, workflow ou activation shortcut exigem confirmação ao trocar aba/aplicativo ou fechar Settings. Aceitar descarte de workflow recarrega a versão salva/rascunho vazio. A janela proprietária continua utilizável com PIN; diálogo de teclado passa pelo pai e alcança os controles da aba incorporada. Enter usa a ação da página, e Escape solicita fechamento.

Texto Markdown armazenado com LF é convertido para CRLF no EDIT nativo e normalizado ao aplicar o Step. Limite do controle acomoda expansão de CRLF; o conteúdo salvo e a validação de anexos continuam limitados a 4096 caracteres. Schema v3, ui-state.ini e anexos existentes preservados.

## Evidências e revisão
- Compilação Release x64 com SkipTests concluída para a reorganização. Nenhum teste adicionado ou executado. Capturador DEMO adaptado apenas para as abas incorporadas.
- Capturas isoladas de General/Shortcuts/Workflows em Dark/Light/System. Inspeção inicial encontrou texto residual de abas anteriores; corrigidos fundos dos Static e repintura de filhos. Confirmação visual Dark/Light mostrou abas sem sobreposição, formulários e ações legíveis. Conversão LF/CRLF acrescentada por inspeção do texto-fonte após essa confirmação; não foi feita rodada extra de polimento visual.
- Revisão independente detectou que confirmar descarte e cancelar Browse mantinha o rascunho editado com flags limpas. Corrigido recarregando o fluxo salvo; revisor confirmou por inspeção. Sem testes ou build executados pelo revisor.
- Atualização final do executável inicialmente bloqueada por app aberto com workflow pendente. Saída automática abortou corretamente; usuário informou fechamento para atualização. Build candidata separada foi preparada durante a espera.
- git diff --check sem erros. Não há alegação de aceitação completa de teclado, reinício, High Contrast ou monitores/DPI.

## Aceitação manual pendente
1. Abrir Settings pela bandeja: General deve mostrar somente preferências globais. Alterar tema e activation shortcut; reabrir/reiniciar e conferir persistência.
2. Shortcuts: escolher aplicativo, selecionar/criar/editar/excluir/recordar atalhos, salvar e executar pela popup. Lista e formulário devem permanecer acessíveis ao redimensionar.
3. Workflows: confirmar mesmo aplicativo selecionado, editar título/Steps, reordenar, anexar imagem, salvar e conferir Markdown na popup. Reabrir e conferir quebras de linha.
4. Com alterações pendentes, trocar aba/app, usar Browse e fechar/Exit. Cancelar descarte deve manter edição e contexto; aceitar deve restaurar dados salvos, inclusive após cancelar Browse.
5. Tab/Shift+Tab entre abas, seletor e editor incorporado; Enter nas páginas e no texto multiline; Escape; PIN e propriedade da janela. Nenhuma ação oculta deve salvar um atalho de outra aba.
6. Conferir Light/Dark/System, High Contrast, tamanho mínimo e diferentes monitores/DPI. Dados pessoais devem permanecer intactos.

## Entrega
feature/0.2.0-settings-tabs → develop → release/0.2.0-alpha.7. Candidata aguardando aceitação, main permanece no release publicado.
