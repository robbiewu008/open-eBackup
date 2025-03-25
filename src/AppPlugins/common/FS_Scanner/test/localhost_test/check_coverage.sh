BRANCH_COV_THRESHOLD=35
FUNCTIONS_COV_THRESHOLD=77
LINES_COV_THRESHOLD=70

function check_coverage()
{
    local branches_cov=$(cat coverage.log | grep branches | awk -F' ' '{print $2}' | awk -F'%' '{print $1}')
    local functions_cov=$(cat coverage.log | grep functions | awk -F' ' '{print $2}' | awk -F'%' '{print $1}')
    local lines_cov=$(cat coverage.log | grep lines | awk -F' ' '{print $2}' | awk -F'%' '{print $1}')
    echo "branches_cov = $branches_cov%, functions_cov = $functions_cov%, lines_cov = $lines_cov%"

    if [ $(echo $branches_cov"<"$BRANCH_COV_THRESHOLD | bc -l) -eq 1 ]
    then
        echo "branch coverage $branches_cov% unqualified, need $BRANCH_COV_THRESHOLD%"
        return 1
    elif [ $(echo $functions_cov"<"$FUNCTIONS_COV_THRESHOLD | bc -l) -eq 1 ]
    then
        echo "functions coverage $functions_cov% unqualified, need $FUNCTIONS_COV_THRESHOLD%"
        return 1
    elif [ $(echo $lines_cov"<"$LINES_COV_THRESHOLD | bc -l) -eq 1 ]
    then
        echo "lines coverage $lines_cov% unqualified, need $LINES_COV_THRESHOLD%"
        return 1
    fi
    echo "check_coveraged passed"
    return 0
}