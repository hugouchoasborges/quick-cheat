# 0.2.0-alpha.5 — Cabeçalho e ícones

## Alterações
Nome do aplicativo ativo maior (19px contra 12px anterior), centralizado numa linha própria. Busca abaixo do título. Limites simétricos mantêm o título no centro com PIN ligado ou desligado; nomes longos usam reticências.

PIN usa alfinete desenhado em GDI, inclinado quando desligado e vertical com destaque quando ligado. Close usa X vermelho, exibido apenas com PIN ativo. Botões nativos de 36px, escalados por DPI, preservam foco por teclado, nomes acessíveis e tooltips. High Contrast usa a cor do sistema para o X. Comportamento de foco, Escape, fechamento na bandeja e dados locais não mudou.

## Evidências
- Release x64 compilada com scripts/build.ps1 -SkipTests; nenhum teste adicionado ou executado. Avisos existentes do parser MD4C upstream permanecem.
- Capturas DEMO isoladas conferidas: popup fixada Dark/Light e Workflows Dark. Título centrado, ícones legíveis, busca e conteúdo sem sobreposição.
- Ajustados os offsets do leitor de fluxos e da grade, inclusive a rotina que reposiciona detalhes dos atalhos.
- git diff --check sem erros. Aceitação manual de clique/teclado, hover, nomes longos, monitores/DPI e High Contrast continua pendente.

## Entrega
feature/0.2.0-popup-header-icons → develop → release/0.2.0-alpha.5. main reservada ao release publicado; candidata aguarda aceitação. Schema v3 e perfis existentes preservados.
