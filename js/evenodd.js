// For table data for alternate row highlighting
class EvenOdd {
    constructor(){
      this.val = -1;
    }
    next(){
        this.val *= -1;
        if (this.val > 0){
            return "odd";
        }else{
            return "even";
        }
    }
}
