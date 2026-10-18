# 0.1.0-alpha.4 — execução por clique duplo

## Decisão e implementação
Pedido de Hugo: clique duplo numa linha da popup também executa o atalho. Clique simples continua selecionando. Execute e Enter continuam disponíveis.
Tratamento nativo de WM_LBUTTONDBLCLK com verificação da linha e uso do mesmo caminho App::execute, mantendo validações do aplicativo de destino. Formato de dados permanece v2.

## Validação e aceitação
Build Release x64 compilada com sucesso via scripts/build.ps1 -SkipTests. Testes automatizados e interação manual não foram executados nesta alteração.
Aceitação pendente: clique simples não executa; clique duplo executa uma vez no destino correto; duplo clique na área vazia não executa; Execute e Enter continuam funcionando; troca de aplicativo não envia ao destino errado.
Gitflow: feature/0.1.0-double-click, develop e release/0.1.0-alpha.4. main permanece reservada ao release publicado.