# 0.2.0-alpha.2 — Contexto preservado e espaço de leitura

## Pedido
Hugo reportou perda do aplicativo anterior ao clicar na interface (No active application) e dificuldade para ler muitos atalhos/fluxos em áreas pequenas com rolagem.

## Diagnóstico e correção
Inspeção identificou que onForeground substituía o destino mesmo quando identify retornava vazio. A identificação pode falhar durante transições, janela nula/inacessível/encerrada. O tratamento de WM_MOUSEACTIVATE também limpava seleção sem verificar se o contexto havia efetivamente mudado.
Agora eventos próprios, nulos, falhas de identificação e superfícies transitórias de bandeja/menu não substituem o último aplicativo válido. Uma mudança real para outro aplicativo continua atualizando a popup. Seleção só é invalidada após uma nova identidade válida. Validação da janela/processo e confirmação de foco antes de SendInput permanecem.
Diagnóstico por inspeção; o cenário exato relatado no computador de Hugo ainda requer confirmação manual após a correção.

## Leitura e interação
- Popup padrão 1040x760 unidades lógicas, limitada à área útil do monitor.
- Redimensionamento nativo pelas bordas, arraste pelo cabeçalho e Expand/Restore.
- Tamanho preferido preservado durante a sessão, sem alterar o schema de dados.
- Workflows em colunas quando há largura suficiente: árvore à esquerda e texto completo à direita; em larguras menores, áreas empilhadas com altura proporcional.
- Maior espaçamento dos passos; rótulos resumidos com reticências e texto completo no leitor, sem barra horizontal da árvore.
- Descrições de atalhos com quebra de linha e altura calculada. Linhas respeitam o limite nativo de 255 pixels físicos; ao atingir o limite, uma área de leitura mostra a descrição completa do atalho selecionado.

## Validação
Build Release x64 compilada com scripts/build.ps1 -SkipTests. Revisão independente resolveu o limite nativo de altura das linhas; correção confirmada por inspeção. Capturas de dados DEMO isolados inspecionadas em Dark/Light, em uma rodada inicial e uma confirmação. Sem testes novos ou execução de suítes nesta alteração.
Amostra isolada em repouso antes da UI: working set 14.65 MiB, private 2.81 MiB, CPU 0 ms em 1 segundo; não representa teto ou benchmark formal.

## Aceitação pendente
1. Ativar ChatGPT, abrir popup e clicar na busca, abas, listas, passos, botões e cabeçalho; contexto deve continuar ChatGPT.
2. Abrir pelo ícone da bandeja e pelo cadastro; não deve aparecer No active application após já haver aplicativo válido.
3. Alternar para outro app real; popup deve mostrar o perfil correto. Fechar o destino não pode enviar teclas a outra janela.
4. Arrastar bordas, usar Expand/Restore, mover pelo cabeçalho e fechar/reabrir; tamanho deve ser mantido durante a sessão e respeitar a área útil.
5. Navegar por muitos atalhos/fluxos, ler texto longo e alternar entre layouts largo/estreito; conferir Light/Dark/System e escalas utilizadas, incluindo 200% se disponível.
6. Conferir clique simples, clique duplo, Execute, Enter na busca e ausência de execução em Workflows.

## Dados e entrega
Schema v3 mantido; atalhos/fluxos locais existentes, inclusive exemplos do ChatGPT, preservados. Sem novo release público até aceitação/publicação autorizada. main permanece no release alpha.4; implementação segue Gitflow em feature/0.2.0-context-and-reading, develop e release/0.2.0-alpha.2.