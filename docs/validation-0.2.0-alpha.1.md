# 0.2.0-alpha.1 — Local Workflows

## Status
Candidata para aceitação manual. A versão pública continua v0.1.0-alpha.4; main não será promovida antes de publicação aprovada.

## Escopo aprovado
Em 2026-10-18, Hugo aprovou começar Fluxos locais pela primeira entrega: título e passos de texto ordenados por aplicativo, consulta expansível na popup e cadastro local. São guias de referência, sem automação de comandos.

## Implementação
- Profile inclui Workflow (título e passos), com limites explícitos: 256 fluxos/app, 128 passos/fluxo, 240 caracteres/título, 4096 caracteres/passo; limites globais e armazenamento de até 8 MiB.
- Cadastro: Manage workflows na biblioteca do aplicativo selecionado; criar, editar, excluir fluxos; inserir, editar, remover e reordenar passos. Save workflow inclui o texto de passo ainda em edição. Confirmação de descarte ao trocar/fechar fluxo ou sair pela bandeja.
- Popup: Shortcuts/Workflows só aparece quando o app tem fluxos. Árvore nativa expansível, seleção por teclado/mouse e texto completo com quebra de linha/rolagem. Busca pelo título. Execute fica oculto e a execução é bloqueada nessa aba.
- Win32 e controles nativos; temas System/Light/Dark; sem polling, rede, frameworks novos ou automação de fluxos.

## Persistência e reversão
Leitura de v1/v2/v3; gravação em schema v3. Primeiro save de arquivo antigo preserva shortcuts.qc.pre-v3.bak (não sobrescrita em saves posteriores), além do backup normal shortcuts.qc.bak. Migração aborta se não puder preservar a cópia. Dados e atalhos existentes ficam no mesmo arquivo.
Releases anteriores não leem v3. Para voltar: fechar o app, copiar o arquivo atual para segurança e restaurar shortcuts.qc.pre-v3.bak como shortcuts.qc. Fluxos e alterações feitas depois da migração não existirão nessa cópia antiga.

## Validação realizada
Build Release Windows x64 concluída com scripts/build.ps1 -SkipTests. Capturas reais geradas por qc_preview usando dados DEMO isolados, inspecionadas em Dark/Light (System também capturado). Sem testes novos ou execução das suítes nesta alteração; cadastro completo, migração e cenários de falha aguardam aceitação.
Amostra única antes de abrir a UI DEMO: working set 14.70 MiB, private 2.82 MiB, CPU 0 ms em 1 segundo; não é teto de consumo nem benchmark formal.

## Aceitação manual proposta
1. Selecionar um app em Manage shortcuts e abrir Manage workflows.
2. Criar fluxo com título e três passos; editar o texto, mover para cima/baixo e remover um passo; salvar.
3. Reabrir o app e confirmar ordem/texto e permanência dos atalhos, tema e hotkey anteriores.
4. Abrir popup, alternar as abas, expandir/recolher e consultar texto longo; trocar o app em foreground.
5. Confirmar que Enter/cliques na aba Workflows não executam atalhos; Shortcuts mantém Execute e clique duplo.
6. Alterar um passo e fechar/trocar fluxo: escolher não descartar mantém a edição. Verificar o mesmo ao sair pela bandeja.
7. Após primeiro save de arquivo v1/v2, conferir shortcuts.qc.pre-v3.bak; saves seguintes não devem sobrescrevê-la.
8. Conferir Light/Dark/System e escalas de tela usadas no dia a dia.

## Limitações
Sem imagens, marcação de concluído, vínculo de atalhos aos passos ou compartilhamento. Expansão/seleção da árvore é temporária e reiniciada ao atualizar contexto/busca. Aceitação manual e publicação desta candidata pendentes.
## Revisão de código
Revisão independente encontrou substituição indevida do rascunho da biblioteca ao salvar/fechar fluxos. Corrigido sincronizando apenas workflows por executável, preservando perfis pendentes, índices e edições de atalhos. Correção confirmada por inspeção pelo revisor; build Release recompilada com sucesso. Nenhum teste executado.
Cenário adicional de aceitação: deixar um perfil novo pendente no cadastro de atalhos, abrir os fluxos de outro app pela popup, salvar/fechar e voltar ao perfil pendente; ele deve continuar válido e editável.